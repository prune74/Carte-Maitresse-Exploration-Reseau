/*
DiscoveryWatchdog_TaskSupervision.cpp

🎯 Rôle
Tâche FreeRTOS chargée d’analyser périodiquement les timeouts des satellites
et de déclencher un STOP d’urgence en cas de silence prolongé.

📌 Fonctionnement
- Boucle infinie FreeRTOS
- Appel à DiscoveryWatchdog_supervise()
- Période WD_PERIOD_MS

📌 Particularités
- Ne dépend pas du protocole Discovery
- Utilise DiscoveryWatchdog_triggerEmergencyStop()
*/

#include "DiscoveryWatchdog_Watchdog.h"

void DiscoveryWatchdog_TaskSupervision(void *pv)
{
    for (;;)
    {
        DiscoveryWatchdog_supervise();
        vTaskDelay(pdMS_TO_TICKS(WD_PERIOD_MS));
    }
}
