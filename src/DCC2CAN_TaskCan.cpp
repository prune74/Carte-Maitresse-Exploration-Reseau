/*
DCC2CAN_TaskCan.cpp / .h

🎯 Rôle
Tâche FreeRTOS responsable de l’envoi des trames CAN Booster. Elle transmet
le bit DCC logique courant (0, 1 ou cutout) ainsi que sa phase, tels qu’ils
sont fournis par le décodeur DCC.

Cette tâche constitue le lien entre le décodeur DCC et le bus CAN Booster.

📌 Fonctionnement
- taskCan() :
    • boucle FreeRTOS exécutée toutes les 2 ms
    • appelle BoosterState_sendCan() pour envoyer :
        - bit DCC courant
        - phase du signal
    • garantit un débit CAN stable et régulier pour les boosters

📌 Particularités
- La tâche est volontairement très légère : aucune logique métier ici.
- Le rythme de 2 ms assure une réactivité suffisante pour suivre le flux DCC.
- L’état DCC est mis à jour par taskDcc() et récupéré via BoosterState.
- Le driver CAN Booster (DCC2CAN_CanBooster) encapsule l’encodage et l’envoi
  des trames CAN.

🔗 Dépendances
- DCC2CAN_State       → accès au dernier bit DCC + phase
- DCC2CAN_CanBooster  → envoi des trames CAN
- FreeRTOS            → gestion de la tâche
*/

#include "DCC2CAN_TaskCan.h"

void taskCan(void *pv)
{
    (void)pv; // évite un warning si pv n'est pas utilisé

    for (;;)
    {
        BoosterState_sendCan();          // envoi du bit DCC courant
        vTaskDelay(pdMS_TO_TICKS(2));    // cadence fixe 2 ms
    }
}
