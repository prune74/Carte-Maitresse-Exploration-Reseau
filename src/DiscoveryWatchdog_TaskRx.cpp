/*
DiscoveryWatchdog_TaskRx.cpp — Version Discovery 2026

🎯 Rôle
Tâche FreeRTOS chargée de lire les trames CAN Discovery contenant les heartbeat
des satellites, et de mettre à jour SatManager via le Watchdog.

📌 Fonctionnement
- Boucle FreeRTOS infinie
- Lecture CAN via canService.getLastFrame()
- Filtrage : ne traite que les heartbeat Discovery (ID 0x200)
- Extraction de l’ID satellite (2 octets)
- Appel à DiscoveryWatchdog_registerHeartbeat()

📌 Particularités
- Période très courte (WD_RX_PERIOD_MS)
- Ne traite que les heartbeat Discovery
- Le Watchdog Master utilise ensuite SatManager pour la supervision
*/

#include "DiscoveryWatchdog_Watchdog.h"
#include "DiscoveryMaster_CanService.h"

extern DiscoveryMaster_CanService canService;

void DiscoveryWatchdog_TaskRx(void *pv)
{
    CANMessage msg;

    for (;;)
    {
        // Lecture non bloquante du dernier message CAN
        if (canService.getLastFrame(msg))
        {
            // Heartbeat Discovery 2026 (ID 0x200)
            if (msg.id == DISCOVERY_CAN_ID_HEARTBEAT)
            {
                // ID satellite codé sur 2 octets
                uint16_t satId = (msg.data[0] << 8) | msg.data[1];

                // Mise à jour du Watchdog (SatManager)
                DiscoveryWatchdog_registerHeartbeat(satId);
            }
        }

        // Période de lecture très courte
        vTaskDelay(pdMS_TO_TICKS(WD_RX_PERIOD_MS));
    }
}
