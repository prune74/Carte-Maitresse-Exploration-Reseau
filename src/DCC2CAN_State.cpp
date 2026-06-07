/*
DCC2CAN_State.cpp / .h

🎯 Rôle
Module centralisant l'état logique du flux DCC pour le module DCC2CAN.
Il stocke le dernier événement DCC (bit, phase, durée, type) ainsi que l'état
de supervision lié à la présence du signal DCC.

📌 Fonctionnement
- g_state :
    • lastEvent      : dernier événement DCC (bit, phase, dt_us, type)
    • status         : état logique (RUNNING, DCC_LOST, RECOVERY)
    • lastEventTime  : timestamp du dernier événement DCC

- BoosterState_updateFromDcc() :
    • copie l'événement DCC (volatile) vers l'état interne + timestamp

- BoosterState_sendCan() :
    • envoie le bit DCC courant via le driver CAN Booster

- BoosterState_supervise() :
    • détecte la perte du signal DCC (timeout)
    • gère le failsafe (DCC_LOST)
    • gère le retour à RUNNING après cooldown (RECOVERY)
*/

#include "DCC2CAN_State.h"
#include "Debug.h"

// ---------------------------------------------------------------------------
// État global
// ---------------------------------------------------------------------------
volatile BoosterRuntimeState g_state = {
    {DCC_EVT_BIT, 0, 0, 0},  // lastEvent
    BSTATE_RUNNING,          // status
    0                        // lastEventTime
};

// Mutex
static StaticSemaphore_t gStateUpdateMutexBuffer;
SemaphoreHandle_t gStateUpdateMutex = nullptr;

// Timer interne pour le recovery
static uint32_t recoveryStartTime = 0;

// ---------------------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------------------
void BoosterState_init() {
    gStateUpdateMutex = xSemaphoreCreateMutexStatic(&gStateUpdateMutexBuffer);
    LOG_INFO("BoosterState → Mutex initialisé");
}

// ---------------------------------------------------------------------------
// Mise à jour depuis un événement DCC (appelé par taskDcc)
// ---------------------------------------------------------------------------
void BoosterState_updateFromDcc(const volatile DccEvent &ev) {
    xSemaphoreTake(gStateUpdateMutex, portMAX_DELAY);

    g_state.lastEvent.bit   = ev.bit;
    g_state.lastEvent.phase = ev.phase;
    g_state.lastEvent.dt_us = ev.dt_us;
    g_state.lastEvent.type  = ev.type;
    g_state.lastEventTime   = millis();

    xSemaphoreGive(gStateUpdateMutex);

    LOG_VERBOSE("BoosterState ← bit=%u phase=%u dt=%lu",
                ev.bit, ev.phase, (unsigned long)ev.dt_us);
}

// ---------------------------------------------------------------------------
// Envoi CAN du bit courant (appelé par taskCan)
// ---------------------------------------------------------------------------
void BoosterState_sendCan() {
    xSemaphoreTake(gStateUpdateMutex, portMAX_DELAY);

    if (g_state.status == BSTATE_RUNNING) {

        uint8_t bit   = g_state.lastEvent.bit;
        uint8_t phase = g_state.lastEvent.phase;

        // 🔥 Anti-spam CAN : n'envoyer que si le bit ou la phase a changé
        static uint8_t lastBit   = 255;
        static uint8_t lastPhase = 255;

        if (bit != lastBit || phase != lastPhase) {
            lastBit   = bit;
            lastPhase = phase;

            CanBooster_sendDccBit(bit, phase);
        } else {
            // LOG_VERBOSE("CAN TX → inchangé, pas d'envoi");
        }

    } else {
        LOG_VERBOSE("BoosterState → CAN TX bloqué (status=%u)", g_state.status);
    }

    xSemaphoreGive(gStateUpdateMutex);
}

// ---------------------------------------------------------------------------
// Supervision du flux DCC (appelé par taskSupervision)
// ---------------------------------------------------------------------------
void BoosterState_supervise() {
    xSemaphoreTake(gStateUpdateMutex, portMAX_DELAY);

    uint32_t now = millis();
    uint32_t timeSinceLastEvent = now - g_state.lastEventTime;

    // -----------------------------------------------------------------------
    // Perte du signal DCC
    // -----------------------------------------------------------------------
    if (timeSinceLastEvent > DCCB_FAILSAFE_TIMEOUT_MS) {

        if (g_state.status == BSTATE_RUNNING) {
            g_state.status = BSTATE_DCC_LOST;
            recoveryStartTime = now;
            LOG_WARN("⚠️  DCC LOST → aucun événement depuis %u ms", timeSinceLastEvent);
        }
        else if (g_state.status == BSTATE_DCC_LOST) {
            if (now - recoveryStartTime > DCCB_FAILSAFE_COOLDOWN_MS) {
                g_state.status = BSTATE_RECOVERY;
                LOG_INFO("DCC → passage en mode RECOVERY");
            }
        }
    }

    // -----------------------------------------------------------------------
    // Signal revenu → retour à RUNNING
    // -----------------------------------------------------------------------
    else {
        if (g_state.status != BSTATE_RUNNING) {
            g_state.status = BSTATE_RUNNING;
            LOG_INFO("✅ DCC RECOVERED → signal revenu");
        }
    }

    xSemaphoreGive(gStateUpdateMutex);
}
