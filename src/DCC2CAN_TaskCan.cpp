/*
DCC2CAN_TaskCan.cpp / .h

🎯 Rôle
Tâche FreeRTOS responsable de l’envoi des trames CAN Booster.
Elle récupère l’état courant du Booster (bit DCC, phase, etc.) et transmet
périodiquement les informations nécessaires au bus CAN interne de l’ESP32.

Cette tâche constitue le lien entre le décodage DCC et le réseau CAN Booster.

📌 Fonctionnement
- taskCan() :
    • boucle FreeRTOS exécutée toutes les 2 ms
    • appelle BoosterState_sendCan() pour envoyer :
        - bit DCC courant
        - phase du signal
        - autres informations selon l’évolution du protocole
    • garantit un débit CAN stable et régulier

📌 Particularités
- La tâche est volontairement très légère : aucune logique métier ici.
- Le rythme de 2 ms assure une réactivité suffisante pour suivre le flux DCC.
- Le module utilise l’état global du Booster (g_state) mis à jour par taskDcc().
- Le driver CAN Booster (DCC2CAN_CanBooster) encapsule toute la logique
  d’encodage et d’envoi des trames.

🔗 Dépendances
- DCC2CAN_State       → accès au dernier événement DCC
- DCC2CAN_CanBooster  → envoi des trames CAN
- FreeRTOS                    → gestion de la tâche
*/

#include "DCC2CAN_TaskCan.h"

void taskCan(void *pv)
{
    for (;;)
    {
        BoosterState_sendCan();
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
