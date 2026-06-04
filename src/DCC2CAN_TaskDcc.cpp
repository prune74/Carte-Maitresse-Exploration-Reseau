/*
DCC2CAN_TaskDcc.cpp / .h

🎯 Rôle
Tâche FreeRTOS dédiée à la consommation des événements DCC décodés par l’ISR.
Elle récupère les événements produits par le module DccDecoder et met à jour
l’état interne du module DCC2CAN (bit, phase, durée). C’est le point de jonction
entre le décodage temps réel du signal DCC et les autres tâches du système.

📌 Fonctionnement
- taskDcc() :
    • boucle FreeRTOS exécutée toutes les 1 ms
    • lit les événements DCC via DccDecoder_getEvent()
    • si un événement est disponible :
        - copie les informations (bit, phase, durée)
          dans l’état global via BoosterState_updateFromDcc()
    • laisse ensuite la main au scheduler FreeRTOS

📌 Particularités
- Le décodage DCC est interrupt-driven : l’ISR remplit une file d’événements.
- Cette tâche ne fait que consommer ces événements, sans logique métier.
- Le traitement est volontairement minimal pour garantir :
    • une latence très faible
    • une réactivité maximale
    • une séparation claire entre décodage, CAN et supervision
- L’état mis à jour ici est utilisé par :
    • taskCan()        → envoi CAN Booster (TX only)
    • taskSupervision()→ logique failsafe

🔗 Dépendances
- DCC2CAN_DccDecoder  → récupération des événements DCC
- DCC2CAN_State       → mise à jour de l’état interne
- FreeRTOS            → gestion de la tâche
*/

#include "DCC2CAN_TaskDcc.h"

void taskDcc(void *pv)
{
    (void)pv; // éviter un warning si pv n'est pas utilisé

    DccEvent ev;

    for (;;)
    {
        // Lire un événement DCC si disponible
        if (DccDecoder_getEvent(ev))
        {
            BoosterState_updateFromDcc(ev);
        }

        // Cadence fixe 1 ms
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
