/*
DCC2CAN_Supervision.cpp / .h

🎯 Rôle
Tâche FreeRTOS dédiée à la supervision du signal DCC. Elle vérifie
périodiquement la présence du flux DCC logique et met à jour l'état de
supervision (RUNNING, DCC_LOST, RECOVERY) via BoosterState_supervise().

📌 Fonctionnement
- taskSupervision() :
    • boucle FreeRTOS exécutée toutes les 20 ms
    • appelle BoosterState_supervise() pour analyser l'état du signal DCC
    • ne contient aucune logique métier : simple scheduler

- BoosterState_supervise() (dans DCC2CAN_State.cpp) :
    • détecte la perte du signal DCC (timeout)
    • gère l'état DCC_LOST (failsafe)
    • gère le retour à RUNNING après le cooldown (RECOVERY)

📌 Particularités
- La supervision est séparée des tâches DCC et CAN pour une architecture claire.
- La fréquence de 20 ms permet une détection rapide sans surcharge CPU.
- Le module ne gère ni télémétrie, ni RailCom, ni mesures analogiques.
- Le mécanisme est purement logique : supervision du flux DCC uniquement.

🔗 Dépendances
- DCC2CAN_State  → logique de supervision DCC
- FreeRTOS       → gestion de la tâche
*/

#include "DCC2CAN_Supervision.h"

void taskSupervision(void *pv)
{
    for (;;)
    {
        BoosterState_supervise();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}
