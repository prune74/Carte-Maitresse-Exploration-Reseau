/*
DiscoveryWatchdog_TaskRx.cpp

🎯 Rôle
Tâche FreeRTOS chargée de lire les trames CAN Discovery contenant les heartbeat
des satellites, et de mettre à jour la table des timestamps Watchdog.

📌 Fonctionnement
- Boucle infinie FreeRTOS
- Lecture CAN via canService.getLastFrame()
- Extraction de l’ID satellite
- Appel à DiscoveryWatchdog_registerHeartbeat()

📌 Particularités
- Période très courte (WD_RX_PERIOD_MS)
- Ne traite que les heartbeat Discovery
*/

#include "DiscoveryWatchdog_Watchdog.h"
#include "DiscoveryMaster_CanService.h"

extern DiscoveryMaster_CanService canService;

void DiscoveryWatchdog_TaskRx(void *pv)
{
    CANMessage msg;

    for (;;)
    {
        if (canService.getLastFrame(msg))
        {
            if (msg.id == DISCOVERY_CAN_ID_HEARTBEAT)
            {
                uint16_t satId = (msg.data[0] << 8) | msg.data[1];
                DiscoveryWatchdog_registerHeartbeat(satId);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(WD_RX_PERIOD_MS));
    }
}
