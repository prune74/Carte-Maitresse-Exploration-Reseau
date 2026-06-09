#include "ExplorationReseau_Maitre_Task.h"
#include "ExplorationReseau_Maitre_SatManager.h"
#include "ExplorationReseau_Maitre_CanService.h"
#include "ExplorationReseau_Maitre_WebHandler.h"
#include "Debug.h"

// --- Instances externes déclarées dans main.cpp ---
extern ERM_SatManager   satManager;
extern ERM_CanService   canService;
extern ERM_WebHandler   webHandler;

/*
 * ExplorationReseau_Maitre_Task.cpp
 *
 * 🎯 Rôle
 * Tâche FreeRTOS dédiée à la supervision périodique du réseau :
 *   • timeouts satellites
 *   • état du bus CAN
 *   • push WebSocket vers l’interface Web
 *
 * Cette tâche tourne à 1 Hz pour garantir une supervision régulière
 * sans surcharger le CPU.
 */

// ---------------------------------------------------------------------------
// CRÉATION DE LA TÂCHE
// ---------------------------------------------------------------------------
void ERM_Task::begin()
{
    LOG_INFO("ERM_Task → création de la tâche (1 Hz)");

    TaskHandle_t taskHandle = NULL;
    xTaskCreate(taskLoop, "ERM_Task", 4096, NULL, 2, &taskHandle);
}

// ---------------------------------------------------------------------------
// BOUCLE DE LA TÂCHE
// ---------------------------------------------------------------------------
// Exécutée à intervalle fixe (1 Hz) grâce à vTaskDelayUntil.
void ERM_Task::taskLoop(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    const uint32_t periodMs   = 1000;  // 1 seconde
    const uint32_t timeoutMs  = 3000;  // 3 s sans heartbeat → offline
    const uint32_t canTimeout = 2000;  // 2 s sans trame CAN → bus KO

    LOG_INFO("ERM_Task → démarrée (timeout=%u ms, CAN timeout=%u ms)",
             timeoutMs, canTimeout);

    for (;;)
    {
        // 1) Vérifier les timeouts des satellites
        satManager.checkTimeouts(timeoutMs);
        LOG_VERBOSE("ERM_Task → supervision satellites OK");

        // 2) Vérifier l’état du CAN
        canService.checkBus(canTimeout);
        LOG_VERBOSE("ERM_Task → supervision CAN OK");

        // 3) Push WebSocket automatique vers l’UI
        webHandler.pushStatus();
        LOG_VERBOSE("ERM_Task → push WebSocket");

        // 4) (Optionnel) Sauvegarde périodique
        // ERM_Settings::writeFile();

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(periodMs));
    }
}
