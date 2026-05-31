/*
DiscoveryWatchdog_TaskSupervision.cpp — Version Discovery 2026

🎯 Rôle
Tâche FreeRTOS chargée d’analyser périodiquement les timeouts des satellites
et de déclencher un STOP global en cas de silence prolongé.

📌 Fonctionnement
- Boucle FreeRTOS infinie
- Appel à DiscoveryWatchdog_supervise()
- Période WD_PERIOD_MS

📌 Particularités
- Ne dépend pas du protocole Discovery
- Utilise SatManager comme source de vérité
- Déclenche DiscoveryWatchdog_triggerEmergencyStop() si nécessaire
*/

#include "DiscoveryWatchdog_Watchdog.h"

void DiscoveryWatchdog_TaskSupervision(void *pv)
{
    for (;;)
    {
        // Analyse des timeouts + STOP global si nécessaire
        DiscoveryWatchdog_supervise();

        // Période de supervision
        vTaskDelay(pdMS_TO_TICKS(WD_PERIOD_MS));
    }
}
