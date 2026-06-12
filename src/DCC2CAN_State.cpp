/*
 * DCC2CAN_State.cpp
 *
 * Rôle
 * ----
 * Module centralisant l’état logique du flux DCC pour la carte maître.
 *
 * Ce composant :
 *   • mémorise le dernier événement DCC reçu
 *   • supervise la présence du signal (failsafe)
 *   • transmet au Booster :
 *        - les bits DCC (bit + phase)
 *        - les événements CUTOUT_START / CUTOUT_END
 *
 * L’objectif est de permettre au Booster de reconstruire le signal DCC
 * et de synchroniser parfaitement la fenêtre RailCom.
 */

#include "DCC2CAN_State.h"
#include "Variables.h"
#include "DCC2CAN_CanBooster.h"
#include "Debug.h"

/* ---------------------------------------------------------------------------
 * INITIALISATION DE L’ÉTAT GLOBAL
 * ------------------------------------------------------------------------- */
void BoosterState_init()
{
    gStateUpdateMutex = xSemaphoreCreateMutexStatic(&gStateUpdateMutexBuffer);
    LOG_INFO("BoosterState → Mutex initialisé");
}

/* ---------------------------------------------------------------------------
 * MISE À JOUR DE L’ÉTAT DEPUIS UN ÉVÉNEMENT DCC
 *
 * Appelée par taskDcc().
 * - copie l’événement dans l’état global
 * - met à jour le timestamp
 * - transmet immédiatement les événements CUTOUT au Booster
 *
 * ⚠️ Cette fonction doit rester ultra légère.
 * ------------------------------------------------------------------------- */
void BoosterState_updateFromDcc(const volatile DccEvent &ev)
{
    xSemaphoreTake(gStateUpdateMutex, portMAX_DELAY);

    g_state.lastEvent.bit   = ev.bit;
    g_state.lastEvent.phase = ev.phase;
    g_state.lastEvent.dt_us = ev.dt_us;
    g_state.lastEvent.type  = ev.type;
    g_state.lastEventTime   = millis();

    xSemaphoreGive(gStateUpdateMutex);

    // Log sécurisé (mode test uniquement)
    LOG_CRITICAL_DCC("BoosterState ← bit=%u phase=%u dt=%lu type=%u",
                     ev.bit, ev.phase, (unsigned long)ev.dt_us, ev.type);

    /* ---------------------------------------------------------------
     * TRANSMISSION IMMÉDIATE DES ÉVÉNEMENTS CUTOUT
     *
     * Le Booster doit recevoir ces événements dès qu’ils sont détectés
     * pour synchroniser parfaitement la fenêtre RailCom.
     * ------------------------------------------------------------- */
    if (ev.type == DCC_EVT_CUTOUT_START)
    {
        CanBooster_sendCutout(0);   // 0 = CUTOUT_START
    }
    else if (ev.type == DCC_EVT_CUTOUT_END)
    {
        CanBooster_sendCutout(1);   // 1 = CUTOUT_END
    }
}

/* ---------------------------------------------------------------------------
 * ENVOI CAN DU BIT COURANT
 *
 * Appelée par taskCan().
 * - envoie le bit DCC courant uniquement si :
 *      • le système est en RUNNING
 *      • le bit ou la phase ont changé (anti-spam)
 *
 * ⚠️ Fonction critique : doit rester minimaliste.
 * ------------------------------------------------------------------------- */
void BoosterState_sendCan()
{
    xSemaphoreTake(gStateUpdateMutex, portMAX_DELAY);

    if (g_state.status == BSTATE_RUNNING)
    {
        uint8_t bit   = g_state.lastEvent.bit;
        uint8_t phase = g_state.lastEvent.phase;

        static uint8_t lastBit   = 255;
        static uint8_t lastPhase = 255;

        // Anti-spam CAN : envoi uniquement si changement
        if (bit != lastBit || phase != lastPhase)
        {
            lastBit   = bit;
            lastPhase = phase;

            CanBooster_sendDccBit(bit, phase);
        }
    }
    else
    {
        LOG_CRITICAL_DCC("BoosterState → CAN TX bloqué (status=%u)",
                         g_state.status);
    }

    xSemaphoreGive(gStateUpdateMutex);
}

/* ---------------------------------------------------------------------------
 * SUPERVISION DU SIGNAL DCC
 *
 * Appelée périodiquement par taskSupervision().
 * - détecte la perte du signal
 * - gère les transitions RUNNING → LOST → RECOVERY
 *
 * Non critique : logs autorisés.
 * ------------------------------------------------------------------------- */
void BoosterState_supervise()
{
    xSemaphoreTake(gStateUpdateMutex, portMAX_DELAY);

    uint32_t now = millis();
    uint32_t dt  = now - g_state.lastEventTime;

    /* Perte du signal */
    if (dt > DCCB_FAILSAFE_TIMEOUT_MS)
    {
        if (g_state.status == BSTATE_RUNNING)
        {
            g_state.status = BSTATE_DCC_LOST;
            recoveryStartTime = now;

            LOG_WARN("⚠️  DCC LOST → aucun événement depuis %u ms", dt);
        }
        else if (g_state.status == BSTATE_DCC_LOST)
        {
            if (now - recoveryStartTime > DCCB_FAILSAFE_COOLDOWN_MS)
            {
                g_state.status = BSTATE_RECOVERY;
                LOG_INFO("DCC → passage en mode RECOVERY");
            }
        }
    }
    /* Signal revenu */
    else
    {
        if (g_state.status != BSTATE_RUNNING)
        {
            g_state.status = BSTATE_RUNNING;
            LOG_INFO("✅ DCC RECOVERED → signal revenu");
        }
    }

    xSemaphoreGive(gStateUpdateMutex);
}
