/*
DiscoveryWatchdog_TaskRx.cpp — Version Discovery 2026

🎯 Rôle
Tâche FreeRTOS chargée de lire les trames CAN Discovery contenant les heartbeat
des satellites, et de mettre à jour SatManager via le Watchdog.
*/

#include "DiscoveryWatchdog_Watchdog.h"
#include "DiscoveryMaster_CanService.h"
#include "ProtocolCAN.h"
#include "CanMsg.h"
#include "Debug.h"

extern DiscoveryMaster_CanService canService;

void DiscoveryWatchdog_TaskRx(void *pv)
{
    LOG_INFO("Watchdog RX → tâche démarrée (période %u ms)", WD_RX_PERIOD_MS);

    CanMsg msg;

    for (;;)
    {
        // Lecture non bloquante du dernier message CAN
        if (canService.getLastFrame(msg))
        {
            // Heartbeat Discovery 2026 (ID 0x200)
            if (ProtocolCAN::isHeartbeat(msg.id))
            {
                uint16_t satId =
                    (uint16_t(msg.data[0]) << 8) |
                    uint16_t(msg.data[1]);

                DiscoveryWatchdog_registerHeartbeat(satId);

                LOG_VERBOSE("Watchdog RX → heartbeat de %u", satId);
            }
        }

        // Période très courte
        vTaskDelay(pdMS_TO_TICKS(WD_RX_PERIOD_MS));
    }
}
