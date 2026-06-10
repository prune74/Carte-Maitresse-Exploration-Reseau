/*
 * DCC2CAN_TaskCan.cpp
 *
 * Tâche FreeRTOS responsable de l’envoi des trames CAN contenant
 * le bit DCC courant (bit logique + phase).
 *
 * Cette tâche constitue le lien direct entre :
 *   - le décodeur DCC (qui produit les événements)
 *   - l’état logique du Booster (géré par BoosterState)
 *   - le bus CAN Booster (via CanBooster_sendDccBit)
 *
 * La cadence fixe de 2 ms garantit un flux CAN stable et régulier,
 * tout en laissant BoosterState_sendCan() décider s’il faut réellement
 * envoyer une trame (anti-spam CAN).
 */

#include "DCC2CAN_TaskCan.h"
#include "DCC2CAN_State.h"
#include "Debug.h"

/* ---------------------------------------------------------------------------
   TÂCHE CAN (TX)
   ---------------------------------------------------------------------------
   Cette tâche tourne toutes les 2 ms et demande à BoosterState d’envoyer
   le bit courant sur le bus CAN. BoosterState_sendCan() applique ensuite
   sa logique interne :
     - envoi uniquement si bit/phase ont changé
     - blocage si le système n’est pas en RUNNING
--------------------------------------------------------------------------- */
void taskCan(void *pv)
{
    (void)pv;

    LOG_INFO("Tâche CAN (TX) démarrée → cadence 2 ms");

    for (;;)
    {
        BoosterState_sendCan();   // envoi du bit DCC courant
        LOG_VERBOSE("CAN TX → bit envoyé");

        vTaskDelay(pdMS_TO_TICKS(2));  // cadence fixe 2 ms
    }
}
