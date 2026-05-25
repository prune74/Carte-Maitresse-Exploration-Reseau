/*
DCC2CAN_State.cpp / .h

🎯 Rôle
Module centralisant l'état interne du Booster.
Il stocke les informations dynamiques nécessaires aux tâches FreeRTOS :
dernier événement DCC, état du CAN Monitor, supervision, etc.

Ce module sert de passerelle entre :
- taskDcc()        → met à jour l'état DCC
- taskCan()        → envoie les trames CAN Booster
- taskSupervision()→ surveille le fonctionnement du Booster

📌 Fonctionnement
- g_state :
    • structure contenant le dernier événement DCC reçu
      (bit, phase, durée, type)
    • status : état du Booster (RUNNING, DCC_LOST, OVERCURRENT, RECOVERY)
    • lastEventTime : timestamp du dernier événement DCC
- canMonitorEnabled / canMonitorFilter :
    • variables globales utilisées par le CLI pour activer
      le sniffer CAN Booster

- BoosterState_init() :
    • Crée le mutex de synchronisation

- BoosterState_updateFromDcc() :
    • copie les champs d'un événement DCC (volatile)
      vers l'état interne du Booster + timestamp actuel
    • thread-safe via mutex

- BoosterState_sendCan() :
    • envoie le bit DCC courant via le driver CAN Booster
    • sauf si l'état est DCC_LOST ou RECOVERY
    • thread-safe via mutex

- BoosterState_supervise() :
    • Détection timeout DCC (500ms)
    • Gestion du failsafe et recovery (1s cooldown)
    • Changement d'état automatique

📌 Particularités
- Thread-safe via mutex (dual-core ESP32 compatible)
- Supervision réactive tous les 20ms
- Failsafe automatique en cas de perte signal DCC
- Recovery automatique quand signal revient

🔗 Dépendances
- DCC2CAN_DccDecoder  → structure DccEvent
- DCC2CAN_CanBooster  → envoi des trames CAN Booster
- DCC2CAN_Cli         → activation du CAN Monitor
- FreeRTOS            → mutex de synchronisation
*/

#include "DCC2CAN_State.h"

volatile bool canMonitorEnabled = false;
volatile int32_t canMonitorFilter = -1;
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

    // Envoyer télémétrie de supervision (pas besoin de mutex ici)
    // TODO: Intégrer mesure réelle de courant/tension
    // CanBooster_sendTelemetry(mA, mV, status);
}
