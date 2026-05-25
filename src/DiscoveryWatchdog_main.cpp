/*
DiscoveryWatchdog_main.cpp

🎯 Rôle
Point d’entrée du module Watchdog Discovery 2026.
Ce fichier centralise l’initialisation du Watchdog et la création des tâches
FreeRTOS associées (réception heartbeat + supervision).

📌 Fonctionnement
- DiscoveryWatchdog_begin() initialise la table des heartbeat.
- Crée les deux tâches FreeRTOS :
    • DiscoveryWatchdog_TaskRx          → réception des heartbeat via CAN
    • DiscoveryWatchdog_TaskSupervision → analyse des timeouts et STOP d’urgence
- Le module est totalement indépendant du module SAMain.

📌 Particularités
- Architecture modulaire : le Watchdog peut être activé ou désactivé
  indépendamment du reste du système.
- Le fichier regroupe toute la logique d’intégration FreeRTOS du Watchdog.
*/

#include "DiscoveryWatchdog_main.h"

void DiscoveryWatchdog_begin()
{
    // Initialisation du module Watchdog
    DiscoveryWatchdog_init();

    // Tâche de réception des heartbeat
    xTaskCreate(
        DiscoveryWatchdog_TaskRx,
        "WD_RX",
        4096,
        NULL,
        6,
        NULL);

    // Tâche de supervision (timeouts)
    xTaskCreate(
        DiscoveryWatchdog_TaskSupervision,
        "WD_SUP",
        4096,
        NULL,
        5,
        NULL);
}
