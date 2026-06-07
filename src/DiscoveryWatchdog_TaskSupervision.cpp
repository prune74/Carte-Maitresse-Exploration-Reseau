/*
DiscoveryWatchdog_TaskSupervision.cpp — Version Discovery 2026

🎯 Rôle
Tâche FreeRTOS chargée d’analyser périodiquement les timeouts des satellites
et de déclencher un STOP global en cas de silence prolongé.
*/

#include "DiscoveryWatchdog_Watchdog.h"
#include "Debug.h"

void DiscoveryWatchdog_TaskSupervision(void *pv)
{
    LOG_INFO("Watchdog SUP → tâche démarrée (période %u ms)", WD_PERIOD_MS);

    for (;;)
    {
        // Analyse des timeouts + STOP global si nécessaire
        DiscoveryWatchdog_supervise();
        LOG_VERBOSE("Watchdog SUP → supervision exécutée");

        // Période de supervision
        vTaskDelay(pdMS_TO_TICKS(WD_PERIOD_MS));
    }
}
