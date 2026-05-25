/*
DCC2CAN_TaskCanRx.cpp / .h

🎯 Rôle
Tâche FreeRTOS dédiée à la réception des trames CAN Booster.
Elle constitue le point d’entrée pour toutes les commandes ou informations
provenant du bus CAN interne de l’ESP32 (CAN Booster).

Actuellement minimaliste, ce module est prévu pour accueillir :
- la réception de commandes externes
- la configuration du Booster via CAN
- la gestion d’un mode sniffer (CAN Monitor)
- la synchronisation entre plusieurs boosters

📌 Fonctionnement
- taskCanRx() :
    • boucle FreeRTOS exécutée toutes les 5 ms
    • future : lecture non bloquante des trames CAN Booster
    • future : décodage et dispatch des messages reçus
    • pour l’instant : simple temporisation

📌 Particularités
- Le bus CAN Booster est totalement indépendant du CAN Discovery (MCP2515).
- La réception CAN est isolée dans une tâche dédiée pour éviter tout blocage
  dans les tâches critiques (DCC et envoi CAN).
- Le module est conçu pour évoluer sans impacter la structure du projet.

🔗 Dépendances
- DCC2CAN_CanBooster  → réception des trames CAN (à implémenter)
- FreeRTOS                    → gestion de la tâche
*/

#include "DCC2CAN_TaskCanRx.h"

void taskCanRx(void *pv)
{
    for (;;)
    {
        // future: réception CAN Booster
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
