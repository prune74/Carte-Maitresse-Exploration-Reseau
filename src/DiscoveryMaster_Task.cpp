#include "DiscoveryMaster_Task.h"


// --- Instances externes déclarées dans main.cpp ---
extern DiscoveryMaster_SatManager satManager;
extern DiscoveryMaster_CanService canService;
extern DiscoveryMaster_WebHandler webHandler;

void DiscoveryMaster_Task::begin()
{
    TaskHandle_t taskHandle = NULL;
    xTaskCreate(taskLoop, "DiscoveryTask", 4096, NULL, 2, &taskHandle);
}

void DiscoveryMaster_Task::taskLoop(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();

    const uint32_t periodMs = 1000;    // 1 seconde
    const uint32_t timeoutMs = 3000;   // 3 secondes sans heartbeat → offline
    const uint32_t canTimeout = 2000;  // 2 secondes sans trame CAN → bus KO

    for (;;)
    {
        // 1) Vérifier les timeouts des satellites
        satManager.checkTimeouts(timeoutMs);

        // 2) (Optionnel) Vérifier l’état du CAN
        canService.checkBus(canTimeout);

        // 3) Push WebSocket automatique vers l’UI
        webHandler.pushStatus();

        // 4) (Optionnel) Sauvegarde périodique
        // DiscoveryMaster_Settings::writeFile();

        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(periodMs));
    }
}
