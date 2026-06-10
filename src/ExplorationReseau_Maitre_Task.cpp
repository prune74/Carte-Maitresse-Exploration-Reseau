/*
 * ExplorationReseau_Maitre_Task.cpp
 *
 * 🎯 Rôle
 * Tâche FreeRTOS dédiée à la supervision périodique du réseau :
 *   • timeouts satellites
 *   • état du bus CAN
 *   • push WebSocket vers l’interface Web
 *
 * Cette tâche tourne à cadence fixe (1 Hz) pour garantir une supervision
 * régulière sans surcharger le CPU.
 */

#include "ExplorationReseau_Maitre_Task.h"
#include "ExplorationReseau_Maitre_SatManager.h"
#include "ExplorationReseau_Maitre_CanService.h"
#include "ExplorationReseau_Maitre_WebHandler.h"
#include "Variables.h"
#include "Debug.h"

/* ---------------------------------------------------------------------------
 * ⚙️ PARAMÈTRES INTERNES DE LA TÂCHE
 * ------------------------------------------------------------------------- */
namespace ERM_TaskParams
{
    static constexpr uint32_t PERIOD_MS     = 1000;  // Cadence 1 Hz
    static constexpr uint32_t TIMEOUT_MS    = 3000;  // 3 s sans heartbeat → offline
    static constexpr uint32_t CAN_TIMEOUT_MS = 2000; // 2 s sans trame CAN → bus KO
}

/* ---------------------------------------------------------------------------
 * 🚀 CRÉATION DE LA TÂCHE
 * ------------------------------------------------------------------------- */
void ERM_Task::begin()
{
    LOG_INFO("ERM_Task → création de la tâche (1 Hz)");

    TaskHandle_t taskHandle = nullptr;
    xTaskCreate(taskLoop, "ERM_Task", 4096, nullptr, 2, &taskHandle);
}

/* ---------------------------------------------------------------------------
 * 🔁 BOUCLE DE LA TÂCHE (1 Hz)
 * ------------------------------------------------------------------------- */
void ERM_Task::taskLoop(void *pvParameters)
{
    (void)pvParameters;

    TickType_t xLastWakeTime = xTaskGetTickCount();

    LOG_INFO("ERM_Task → démarrée (timeout=%u ms, CAN timeout=%u ms)",
             ERM_TaskParams::TIMEOUT_MS,
             ERM_TaskParams::CAN_TIMEOUT_MS);

    for (;;)
    {
        /* -----------------------------------------------------------
         * 1) Supervision des satellites
         * --------------------------------------------------------- */
        satManager.checkTimeouts(ERM_TaskParams::TIMEOUT_MS);

        /* -----------------------------------------------------------
         * 2) Supervision du bus CAN
         * --------------------------------------------------------- */
        canService.checkBus(ERM_TaskParams::CAN_TIMEOUT_MS);

        /* -----------------------------------------------------------
         * 3) Push WebSocket vers l’interface Web
         * --------------------------------------------------------- */
        webHandler.pushStatus();

        /* -----------------------------------------------------------
         * 4) Cadence fixe (1 Hz)
         * --------------------------------------------------------- */
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(ERM_TaskParams::PERIOD_MS));
    }
}
