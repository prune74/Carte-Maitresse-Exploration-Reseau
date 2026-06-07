/*
DiscoveryWatchdog_main.cpp — Version Discovery 2026

🎯 Rôle
Point d’entrée du module Watchdog Discovery 2026.
Ce fichier initialise le Watchdog et crée les tâches FreeRTOS associées :
- TaskRx : réception des heartbeat via CAN
- TaskSupervision : analyse des timeouts et STOP d’urgence
*/

#include "DiscoveryWatchdog_main.h"
#include "Debug.h"

void DiscoveryWatchdog_begin()
{
    LOG_INFO("Watchdog → initialisation du module");

    // Initialisation interne du Watchdog
    DiscoveryWatchdog_init();
    LOG_INFO("Watchdog → structures internes initialisées");

    // -----------------------------------------------------------------------
    // Tâche de réception des heartbeat
    // -----------------------------------------------------------------------
    BaseType_t ok1 = xTaskCreate(
        DiscoveryWatchdog_TaskRx,
        "WD_RX",
        4096,
        NULL,
        6,
        NULL);

    if (ok1 == pdPASS)
        LOG_INFO("Watchdog → tâche WD_RX créée (prio 6)");
    else
        LOG_ERROR("Watchdog → échec création tâche WD_RX");

    // -----------------------------------------------------------------------
    // Tâche de supervision (timeouts)
    // -----------------------------------------------------------------------
    BaseType_t ok2 = xTaskCreate(
        DiscoveryWatchdog_TaskSupervision,
        "WD_SUP",
        4096,
        NULL,
        5,
        NULL);

    if (ok2 == pdPASS)
        LOG_INFO("Watchdog → tâche WD_SUP créée (prio 5)");
    else
        LOG_ERROR("Watchdog → échec création tâche WD_SUP");

    LOG_INFO("Watchdog → démarré");
}
