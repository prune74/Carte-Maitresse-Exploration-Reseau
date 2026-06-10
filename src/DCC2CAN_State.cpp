/*
 * DCC2CAN_State.cpp
 *
 * Module centralisant l’état logique du flux DCC pour le module DCC2CAN.
 *
 * Ce composant joue un rôle clé dans la chaîne de traitement :
 *   - il mémorise le dernier événement DCC reçu
 *   - il supervise la présence du signal DCC (failsafe)
 *   - il fournit le bit courant au module CAN Booster
 *
 * Toutes les variables globales (état, mutex, compteurs…) sont centralisées
 * dans Variables.h/.cpp afin d’assurer une architecture claire,
 * modulaire et facilement testable.
 */

#include "DCC2CAN_State.h"
#include "Variables.h"
#include "DCC2CAN_CanBooster.h"
#include "Debug.h"

/* ---------------------------------------------------------------------------
   INITIALISATION DE L’ÉTAT DU BOOSTER
   ---------------------------------------------------------------------------
   Le mutex protège l’accès à l’état global g_state, partagé entre :
     - taskDcc()        → mise à jour depuis les événements DCC
     - taskCan()        → envoi du bit courant sur le bus CAN
     - taskSupervision() → surveillance du signal DCC
--------------------------------------------------------------------------- */
void BoosterState_init()
{
    gStateUpdateMutex = xSemaphoreCreateMutexStatic(&gStateUpdateMutexBuffer);
    LOG_INFO("BoosterState → Mutex initialisé");
}

/* ---------------------------------------------------------------------------
   MISE À JOUR DE L’ÉTAT DEPUIS UN ÉVÉNEMENT DCC
   ---------------------------------------------------------------------------
   Cette fonction est appelée par taskDcc() lorsqu’un événement DCC est reçu.
   Elle copie l’événement dans l’état global et met à jour le timestamp.
--------------------------------------------------------------------------- */
void BoosterState_updateFromDcc(const volatile DccEvent &ev)
{
    xSemaphoreTake(gStateUpdateMutex, portMAX_DELAY);

    g_state.lastEvent.bit = ev.bit;
    g_state.lastEvent.phase = ev.phase;
    g_state.lastEvent.dt_us = ev.dt_us;
    g_state.lastEvent.type = ev.type;
    g_state.lastEventTime = millis();

    xSemaphoreGive(gStateUpdateMutex);

    LOG_VERBOSE("BoosterState ← bit=%u phase=%u dt=%lu",
                ev.bit, ev.phase, (unsigned long)ev.dt_us);
}

/* ---------------------------------------------------------------------------
   ENVOI CAN DU BIT COURANT
   ---------------------------------------------------------------------------
   Cette fonction est appelée par taskCan().
   Elle envoie le bit DCC courant au Booster, uniquement si :
     - le système est en mode RUNNING
     - le bit ou la phase ont changé (anti-spam CAN)
--------------------------------------------------------------------------- */
void BoosterState_sendCan()
{
    xSemaphoreTake(gStateUpdateMutex, portMAX_DELAY);

    if (g_state.status == BSTATE_RUNNING)
    {
        uint8_t bit = g_state.lastEvent.bit;
        uint8_t phase = g_state.lastEvent.phase;

        static uint8_t lastBit = 255;
        static uint8_t lastPhase = 255;

        if (bit != lastBit || phase != lastPhase)
        {
            lastBit = bit;
            lastPhase = phase;

            CanBooster_sendDccBit(bit, phase);
        }
    }
    else
    {
        LOG_VERBOSE("BoosterState → CAN TX bloqué (status=%u)", g_state.status);
    }

    xSemaphoreGive(gStateUpdateMutex);
}

/* ---------------------------------------------------------------------------
   SUPERVISION DU SIGNAL DCC
   ---------------------------------------------------------------------------
   Cette fonction est appelée périodiquement par taskSupervision().
   Elle détecte :
     - la perte du signal DCC (timeout)
     - la transition vers DCC_LOST
     - la transition vers RECOVERY
     - le retour à RUNNING lorsque le signal revient
--------------------------------------------------------------------------- */
void BoosterState_supervise()
{
    xSemaphoreTake(gStateUpdateMutex, portMAX_DELAY);

    uint32_t now = millis();
    uint32_t dt = now - g_state.lastEventTime;

    /* -----------------------------------------------------------------------
       PERTE DU SIGNAL DCC
       ----------------------------------------------------------------------- */
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

    /* -----------------------------------------------------------------------
       SIGNAL REVENUE → RETOUR À RUNNING
       ----------------------------------------------------------------------- */
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
