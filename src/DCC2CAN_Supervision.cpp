/*
DCC2CAN_Supervision.cpp / .h

🎯 Rôle
Tâche FreeRTOS dédiée à la supervision du Booster.
Elle exécute périodiquement la logique de sécurité et de surveillance du module :
perte du signal DCC, surintensité, coupure prolongée, état du booster, etc.

Ce module constitue la couche "failsafe" du système DCC_CAN-Booster.

📌 Fonctionnement
- taskSupervision() :
    • boucle FreeRTOS exécutée toutes les 20 ms
    • appelle BoosterState_supervise() pour analyser l’état courant
    • ne contient aucune logique métier : simple scheduler

- BoosterState_supervise() (dans DCC2CAN_State.cpp) :
    • point d’entrée pour :
        - détection de perte du signal DCC
        - gestion du failsafe
        - surveillance du courant / tension
        - état du booster (OK, OFF, FAULT…)
        - logique RailCom (si nécessaire)
    • actuellement minimal, prévu pour extension

📌 Particularités
- La supervision est séparée des tâches DCC et CAN pour garantir
  une architecture claire et modulaire.
- La fréquence de 20 ms permet une surveillance réactive sans surcharger
  le CPU ni le bus CAN.
- Le module ne dépend que de l’état global du Booster (g_state).

🔗 Dépendances
- DCC2CAN_State  → accès à l’état du Booster
- FreeRTOS               → gestion de la tâche
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
