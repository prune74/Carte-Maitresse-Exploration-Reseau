/*
DCC2CAN_State.cpp / .h

🎯 Rôle
Module centralisant l'état logique du flux DCC pour le module DCC2CAN.
Il stocke le dernier événement DCC (bit, phase, durée, type) ainsi que l'état
de supervision lié à la présence du signal DCC.

Ce module sert de passerelle entre :
- taskDcc()         → met à jour l'état DCC (g_state.lastEvent)
- taskCan()         → envoie le bit DCC courant sur le CAN Booster (TX only)
- taskSupervision() → surveille la présence du signal DCC (failsafe)

📌 Fonctionnement
- g_state :
    • lastEvent      : dernier événement DCC (bit, phase, dt_us, type)
    • status         : état logique (RUNNING, DCC_LOST, RECOVERY)
    • lastEventTime  : timestamp du dernier événement DCC

- BoosterState_init() :
    • initialise le mutex de synchronisation

- BoosterState_updateFromDcc() :
    • copie l'événement DCC (volatile) vers l'état interne + timestamp

- BoosterState_sendCan() :
    • envoie le bit DCC courant via le driver CAN Booster
      (désactivé en cas de DCC_LOST ou RECOVERY)

- BoosterState_supervise() :
    • détecte la perte du signal DCC (timeout)
    • gère le failsafe (DCC_LOST)
    • gère le retour à RUNNING après cooldown (RECOVERY)

📌 Particularités
- Thread-safe via mutex (compatible dual-core ESP32)
- Supervision périodique exécutée par taskSupervision()
- Ne gère ni télémétrie, ni RailCom, ni monitoring CAN
- Supervision purement logique du flux DCC
*/

#include "DCC2CAN_State.h"

volatile BoosterRuntimeState g_state = {
    {DCC_EVT_BIT, 0, 0, 0},  // lastEvent
    BSTATE_RUNNING,           // status
    0                          // lastEventTime
};

static StaticSemaphore_t gStateUpdateMutexBuffer;
SemaphoreHandle_t gStateUpdateMutex;

static uint32_t recoveryStartTime = 0;

void BoosterState_init()
{
    // Initialiser le mutex
    gStateUpdateMutex = xSemaphoreCreateMutexStatic(&gStateUpdateMutexBuffer);
}

void BoosterState_updateFromDcc(const volatile DccEvent &ev)
{
    xSemaphoreTake(gStateUpdateMutex, portMAX_DELAY);
    g_state.lastEvent.bit = ev.bit;
    g_state.lastEvent.phase = ev.phase;
    g_state.lastEvent.dt_us = ev.dt_us;
    g_state.lastEvent.type = ev.type;
    g_state.lastEventTime = millis();
    xSemaphoreGive(gStateUpdateMutex);
}

void BoosterState_sendCan()
{
    xSemaphoreTake(gStateUpdateMutex, portMAX_DELAY);
    // Ne pas envoyer si perte de signal DCC
    if (g_state.status != BSTATE_DCC_LOST && g_state.status != BSTATE_RECOVERY)
    {
        CanBooster_sendDccBit(g_state.lastEvent.bit, g_state.lastEvent.phase);
    }
    xSemaphoreGive(gStateUpdateMutex);
}

void BoosterState_supervise()
{
    xSemaphoreTake(gStateUpdateMutex, portMAX_DELAY);
    
    uint32_t now = millis();
    uint32_t timeSinceLastEvent = now - g_state.lastEventTime;

    // Détection timeout DCC
    if (timeSinceLastEvent > DCCB_FAILSAFE_TIMEOUT_MS)
    {
        if (g_state.status == BSTATE_RUNNING)
        {
            // Perte du signal DCC détectée
            g_state.status = BSTATE_DCC_LOST;
            recoveryStartTime = now;
#ifdef DEBUG
            debug.println("⚠️  DCC LOST - Failsafe engaged");
#endif
        }
        else if (g_state.status == BSTATE_DCC_LOST)
        {
            // En attente de recovery
            if (now - recoveryStartTime > DCCB_FAILSAFE_COOLDOWN_MS)
            {
                // Timeout de cooldown écoulé, passer en recovery mode
                g_state.status = BSTATE_RECOVERY;
            }
        }
    }
    else
    {
        // Signal DCC revenu - sortir de DCC_LOST et RECOVERY
        if (g_state.status == BSTATE_DCC_LOST || g_state.status == BSTATE_RECOVERY)
        {
            g_state.status = BSTATE_RUNNING;
#ifdef DEBUG
            debug.println("✅ DCC RECOVERED");
#endif
        }
    }

    xSemaphoreGive(gStateUpdateMutex);
}
