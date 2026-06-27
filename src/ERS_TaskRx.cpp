/*
 * ERS_TaskRx.cpp
 *
 * 🎯 Rôle
 * Tâche FreeRTOS chargée de lire les trames CAN contenant les heartbeats
 * des Canton Controllers, et de mettre à jour le module de Surveillance ERS.
 *
 * Cette tâche tourne à haute fréquence pour garantir une détection rapide
 * des pertes de communication. Elle doit rester légère, non bloquante,
 * et ne jamais perturber les tâches temps réel critiques (DCC2CAN).
 */

#include "ERS_Watchdog.h"
#include "Variables.h"
#include "ERM_CanService.h"
#include "ProtocolCAN.h"
#include "CanMsg.h"
#include "Debug.h"

// Service CAN principal (déclaré dans main)
extern ERM_CanService canService;

void ERS_TaskRx(void *pv)
{
    (void)pv;

    LOG_INFO("ERS RX → tâche démarrée (période %u ms)", ERS_RX_PERIOD_MS);

    CanMsg msg;

    for (;;)
    {
        /* -----------------------------------------------------------
         * Lecture non bloquante du dernier message CAN
         * --------------------------------------------------------- */
        if (canService.getLastFrame(msg))
        {
            // Heartbeat ERM (ID 0x200)
            if (ProtocolCAN::isHeartbeat(msg.id))
            {
                uint16_t ccId =
                    (uint16_t(msg.data[0]) << 8) |
                    uint16_t(msg.data[1]);

                ERS_registerHeartbeat(ccId);

                // Log sécurisé : jamais en mode réel
                LOG_CRITICAL_DCC("ERS RX → heartbeat de %u", ccId);
            }
        }

        /* -----------------------------------------------------------
         * Cadence fixe (haute fréquence)
         * --------------------------------------------------------- */
        vTaskDelay(pdMS_TO_TICKS(ERS_RX_PERIOD_MS));
    }
}
