/*
DCC2CAN_TaskCan.cpp / .h

🎯 Rôle
Tâche FreeRTOS responsable de l’envoi des trames CAN Booster. Elle transmet
le bit DCC logique courant (0, 1 ou cutout) ainsi que sa phase, tels qu’ils
sont fournis par le décodeur DCC.

Cette tâche constitue le lien entre le décodeur DCC et le bus CAN Booster.
*/

#include "DCC2CAN_TaskCan.h"
#include "DCC2CAN_State.h"
#include "Debug.h"   // 🔥 Ajout du système de logs

void taskCan(void *pv)
{
    (void)pv; // éviter un warning si pv n'est pas utilisé

    LOG_INFO("Tâche CAN (TX) démarrée → cadence 2 ms");

    for (;;)
    {
        BoosterState_sendCan();   // envoi du bit DCC courant
        LOG_VERBOSE("CAN TX → bit envoyé");

        vTaskDelay(pdMS_TO_TICKS(2));  // cadence fixe 2 ms
    }
}
