/*
 * ExplorationReseau_Surveillance_TaskRx.cpp
 *
 * 🎯 Rôle
 * Tâche FreeRTOS chargée de lire les trames CAN contenant les heartbeats
 * des satellites, et de mettre à jour le module de Surveillance ERS.
 *
 * Cette tâche tourne à haute fréquence pour garantir une détection rapide
 * des pertes de communication.
 */

#include "ExplorationReseau_Surveillance_Watchdog.h"
#include "Variables.h"
#include "ExplorationReseau_Maitre_CanService.h"
#include "ProtocolCAN.h"
#include "CanMsg.h"
#include "Debug.h"

// Service CAN principal (déclaré dans main)
extern ERM_CanService canService;

void ERS_TaskRx(void *pv)
{
    LOG_INFO("ERS RX → tâche démarrée (période %u ms)", ERS_RX_PERIOD_MS);

    CanMsg msg;

    for (;;)
    {
        // Lecture non bloquante du dernier message CAN
        if (canService.getLastFrame(msg))
        {
            // Heartbeat ERM (ID 0x200)
            if (ProtocolCAN::isHeartbeat(msg.id))
            {
                uint16_t satId =
                    (uint16_t(msg.data[0]) << 8) |
                    uint16_t(msg.data[1]);

                ERS_registerHeartbeat(satId);

                LOG_VERBOSE("ERS RX → heartbeat de %u", satId);
            }
        }

        // Période très courte
        vTaskDelay(pdMS_TO_TICKS(ERS_RX_PERIOD_MS));
    }
}
