#include "DiscoveryMaster_Task.h"
#include "Debug.h"

// --- Instances externes déclarées dans main.cpp ---
extern DiscoveryMaster_SatManager satManager;
extern DiscoveryMaster_CanService canService;
extern DiscoveryMaster_WebHandler webHandler;

void DiscoveryMaster_Task::begin()
{
    LOG_INFO("DiscoveryTask → création de la tâche (1 Hz)");

    TaskHandle_t taskHandle = NULL;
    xTaskCreate(taskLoop, "DiscoveryTask", 4096, NULL, 2, &taskHandle);
}

void DiscoveryMaster_Task::taskLoop(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    const uint32_t periodMs   = 1000;  // 1 seconde
    const uint32_t timeoutMs  = 3000;  // 3 s sans heartbeat → offline
    const uint32_t canTimeout = 2000;  // 2 s sans trame CAN → bus KO

    LOG_INFO("DiscoveryTask → démarrée (timeout=%u ms, CAN timeout=%u ms)",
             timeoutMs, canTimeout);

    for (;;)
    {
        // 1) Vérifier les timeouts des satellites
        satManager.checkTimeouts(timeoutMs);
        LOG_VERBOSE("DiscoveryTask → supervision satellites OK");

        // 2) Vérifier l’état du CAN
        canService.checkBus(canTimeout);
        LOG_VERBOSE("DiscoveryTask → supervision CAN OK");

        // 3) Push WebSocket automatique vers l’UI
        webHandler.pushStatus();
        LOG_VERBOSE("DiscoveryTask → push WebSocket");

        // 4) (Optionnel) Sauvegarde périodique
        // DiscoveryMaster_Settings::writeFile();

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(periodMs));
    }
}
