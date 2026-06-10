/*
 * DCC2CAN_State.cpp
 *
 * 🎯 Rôle
 * Module centralisant l’état logique du flux DCC pour le module DCC2CAN.
 *
 * Ce composant joue un rôle clé dans la chaîne de traitement :
 *   • mémorise le dernier événement DCC reçu
 *   • supervise la présence du signal DCC (failsafe)
 *   • fournit le bit courant au module CAN Booster
 *
 * Toutes les variables globales (état, mutex, compteurs…) sont regroupées
 * dans Variables.h/.cpp pour garantir une architecture claire, modulaire
 * et facilement testable.
 */

#include "DCC2CAN_State.h"
#include "Variables.h"
#include "DCC2CAN_CanBooster.h"
#include "Debug.h"

/* ---------------------------------------------------------------------------
 * 🧩 INITIALISATION DE L’ÉTAT DU BOOSTER
 *
 * Le mutex protège l’accès à g_state, partagé entre :
 *   • taskDcc()         → mise à jour depuis les événements DCC
 *   • taskCan()         → envoi du bit courant sur le bus CAN
 *   • taskSupervision() → surveillance du signal DCC
 * ------------------------------------------------------------------------- */
void BoosterState_init()
{
    gStateUpdateMutex = xSemaphoreCreateMutexStatic(&gStateUpdateMutexBuffer);
    LOG_INFO("BoosterState → Mutex initialisé");
}

/* ---------------------------------------------------------------------------
 * 🔄 MISE À JOUR DE L’ÉTAT DEPUIS UN ÉVÉNEMENT DCC
 *
 * Appelée par taskDcc() lorsqu’un événement DCC est reçu.
 * Copie l’événement dans l’état global et met à jour le timestamp.
 *
 * ⚠️ Zone critique : aucun log non protégé ici.
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

    // Log sécurisé : actif uniquement en mode test + VERBOSE
    LOG_CRITICAL_DCC("BoosterState ← bit=%u phase=%u dt=%lu",
                     ev.bit, ev.phase, (unsigned long)ev.dt_us);
}

/* ---------------------------------------------------------------------------
 * 📡 ENVOI CAN DU BIT COURANT
 *
 * Appelée par taskCan().
 * Envoie le bit DCC courant au Booster uniquement si :
 *   • le système est en RUNNING
 *   • le bit ou la phase ont changé (anti-spam CAN)
 *
 * ⚠️ Zone critique : doit être ultra légère.
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
 * 🛡️ SUPERVISION DU SIGNAL DCC
 *
 * Appelée périodiquement par taskSupervision().
 * Détecte :
 *   • perte du signal (timeout)
 *   • passage en DCC_LOST
 *   • passage en RECOVERY
 *   • retour à RUNNING
 *
 * ⚠️ Non critique : logs autorisés.
 * ------------------------------------------------------------------------- */
void BoosterState_supervise()
{
    xSemaphoreTake(gStateUpdateMutex, portMAX_DELAY);

    uint32_t now = millis();
    uint32_t dt  = now - g_state.lastEventTime;

    /* -------------------------------
     * PERTE DU SIGNAL DCC
     * ----------------------------- */
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

    /* -------------------------------
     * SIGNAL REVENUE → RETOUR À RUNNING
     * ----------------------------- */
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
