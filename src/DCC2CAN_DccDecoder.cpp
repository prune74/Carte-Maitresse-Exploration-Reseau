/*
 * DCC2CAN_DccDecoder.cpp
 *
 * 🎯 Rôle
 * Module chargé du décodage du signal DCC en temps réel.
 * Il analyse les fronts du signal DCC, détecte les bits, les cutouts,
 * et génère des événements envoyés dans une queue FreeRTOS.
 *
 * Conception :
 *   • ISR ultra rapide (IRAM)
 *   • aucun traitement lourd dans l’ISR
 *   • robustesse face aux timings irréguliers
 *   • compatibilité NMRA
 *
 * Toutes les variables critiques (timings, états, statistiques)
 * sont centralisées dans Variables.h/.cpp.
 */

#include "DCC2CAN_DccDecoder.h"
#include "DCC2CAN_Config.h"
#include "Variables.h"
#include "DCC2CAN_FakeDcc.h"
#include "Debug.h"

// Mode test (FakeDCC) défini globalement
extern bool g_isTestMode;

/* ---------------------------------------------------------------------------
 * ENVOI D’UN ÉVÉNEMENT DEPUIS L’ISR
 *
 * L’ISR ne fait aucun traitement lourd : elle se contente de pousser un
 * événement dans la queue. Cela garantit un fonctionnement stable même
 * à haute fréquence.
 * ------------------------------------------------------------------------- */
static void IRAM_ATTR sendEventFromISR(const DccEvent &ev)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xQueueSendFromISR(dccQueue, (void *)&ev, &xHigherPriorityTaskWoken);

    if (xHigherPriorityTaskWoken == pdTRUE)
        portYIELD_FROM_ISR();
}

/* ---------------------------------------------------------------------------
 * ISR DCC : appelée sur chaque front du signal DCC
 *
 * L’ISR mesure le temps entre deux fronts (dt) et détermine :
 *   • bit 1
 *   • bit 0
 *   • début de cutout
 *   • fin de cutout
 *
 * ⚠️ Aucun log, aucun mutex, aucun traitement lourd ici.
 * ------------------------------------------------------------------------- */
static void IRAM_ATTR dccISR()
{
    uint32_t now = micros();
    uint32_t dt  = now - dcc_lastEdge;
    dcc_lastEdge = now;

    DccEvent ev;
    ev.dt_us = dt;
    ev.phase = dcc_currentPhase;
    ev.bit   = 0;

    /* -------------------------------
     * DÉBUT DE CUTOUT
     * ----------------------------- */
    if (dt > DCCB_TIMING_CUTOUT_START_US)
    {
        if (!dcc_inCutout)
        {
            dcc_inCutout = true;
            dcc_cutoutCount++;

            ev.type = DCC_EVT_CUTOUT_START;
            sendEventFromISR(ev);
        }
        return;
    }

    /* -------------------------------
     * FIN DE CUTOUT
     * ----------------------------- */
    if (dcc_inCutout)
    {
        dcc_inCutout = false;

        ev.type = DCC_EVT_CUTOUT_END;
        sendEventFromISR(ev);
        return;
    }

    /* -------------------------------
     * BIT 1
     * ----------------------------- */
    if (dt >= DCCB_TIMING_BIT1_MIN_US && dt <= DCCB_TIMING_BIT1_MAX_US)
    {
        dcc_currentPhase ^= 1;

        ev.type  = DCC_EVT_BIT;
        ev.bit   = 1;
        ev.phase = dcc_currentPhase;

        dcc_bit1Count++;
        sendEventFromISR(ev);
        return;
    }

    /* -------------------------------
     * BIT 0
     * ----------------------------- */
    if (dt >= DCCB_TIMING_BIT0_MIN_US && dt <= DCCB_TIMING_BIT0_MAX_US)
    {
        dcc_currentPhase ^= 1;

        ev.type  = DCC_EVT_BIT;
        ev.bit   = 0;
        ev.phase = dcc_currentPhase;

        dcc_bit0Count++;
        sendEventFromISR(ev);
        return;
    }

    /* -------------------------------
     * TIMING INVALIDE
     * ----------------------------- */
    dcc_badTiming++;
}

/* ---------------------------------------------------------------------------
 * INITIALISATION DU DÉCODEUR
 *
 * Crée la queue d’événements et, en mode réel, attache l’ISR au pin DCC.
 * En mode test, l’ISR est désactivée et FakeDCC génère les événements.
 * ------------------------------------------------------------------------- */
void DccDecoder_begin()
{
    dccQueue = xQueueCreate(DCC_EVENT_QUEUE_SIZE, sizeof(DccEvent));

    if (dccQueue == nullptr)
        LOG_ERROR("DCC Decoder → échec création de la queue");
    else
        LOG_INFO("DCC Decoder → queue créée (taille=%u)", DCC_EVENT_QUEUE_SIZE);

    // Mode TEST : pas d’ISR, FakeDCC génère les événements
    if (g_isTestMode)
    {
        LOG_WARN("DCC Decoder → mode TEST : ISR désactivée, FakeDCC actif");
        return;
    }

    pinMode(PIN_DCC_IN, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_DCC_IN), dccISR, CHANGE);

    LOG_INFO("DCC Decoder → attachInterrupt sur PIN_DCC_IN=%d", PIN_DCC_IN);
}

/* ---------------------------------------------------------------------------
 * LECTURE D’UN ÉVÉNEMENT DCC
 *
 * Fonction appelée par taskDcc() pour récupérer un événement depuis la queue.
 * Aucun blocage : timeout 0.
 *
 * Les logs détaillés sont protégés via LOG_CRITICAL_DCC pour ne jamais
 * perturber le mode réel.
 * ------------------------------------------------------------------------- */
bool DccDecoder_getEvent(DccEvent &ev)
{
    if (dccQueue == nullptr)
        return false;

    bool ok = xQueueReceive(dccQueue, &ev, 0) == pdTRUE;

    if (ok)
    {
        if (ev.type == DCC_EVT_BIT)
        {
            LOG_CRITICAL_DCC("DCC Event → BIT=%u phase=%u dt=%lu",
                             ev.bit, ev.phase, (unsigned long)ev.dt_us);
        }
        else if (ev.type == DCC_EVT_CUTOUT_START)
        {
            LOG_CRITICAL_DCC("DCC Event → CUTOUT_START dt=%lu",
                             (unsigned long)ev.dt_us);
        }
        else if (ev.type == DCC_EVT_CUTOUT_END)
        {
            LOG_CRITICAL_DCC("DCC Event → CUTOUT_END dt=%lu",
                             (unsigned long)ev.dt_us);
        }
    }

    return ok;
}

/* ---------------------------------------------------------------------------
 * LECTURE DES STATISTIQUES
 *
 * Fonction non critique : logs INFO autorisés.
 * ------------------------------------------------------------------------- */
void DccDecoder_getStats(uint32_t &bit0, uint32_t &bit1,
                         uint32_t &cutout, uint32_t &bad)
{
    bit0   = dcc_bit0Count;
    bit1   = dcc_bit1Count;
    cutout = dcc_cutoutCount;
    bad    = dcc_badTiming;

    LOG_INFO("DCC Stats → b0=%lu b1=%lu cutout=%lu bad=%lu",
             bit0, bit1, cutout, bad);
}

/* ---------------------------------------------------------------------------
 * ACCÈS À LA QUEUE DCC
 * ------------------------------------------------------------------------- */
QueueHandle_t DccDecoder_getQueue()
{
    return dccQueue;
}
