/*
 * ExplorationReseau_Surveillance_main.cpp
 *
 * 🎯 Rôle
 * Point d’entrée du module de Surveillance ERS.
 *
 * Ce module initialise :
 *   • la tâche de réception des heartbeats (ERS_TaskRx)
 *   • la tâche de supervision des timeouts (ERS_TaskSupervision)
 *
 * Il constitue la couche Watchdog du système ERM.
 */

#include "ExplorationReseau_Surveillance_main.h"
#include "ExplorationReseau_Surveillance_Watchdog.h"
#include "Variables.h"
#include "Debug.h"

// ---------------------------------------------------------------------------
// INITIALISATION DU MODULE ERS
// ---------------------------------------------------------------------------
void ERS_begin()
{
    LOG_INFO("ERS → initialisation du module Surveillance");

    // Initialisation interne (mutex, tableaux, compteurs…)
    ERS_init();
    LOG_INFO("ERS → structures internes initialisées");

    // -----------------------------------------------------------------------
    // Tâche de réception des heartbeats
    // Priorité 4 → importante mais ne doit jamais préempter taskDcc (prio 5)
    // -----------------------------------------------------------------------
    BaseType_t ok1 = xTaskCreate(
        ERS_TaskRx,
        "ERS_RX",
        4096,
        nullptr,
        4,          // ★ priorité corrigée
        nullptr);

    if (ok1 == pdPASS)
        LOG_INFO("ERS → tâche ERS_RX créée (prio 4)");
    else
        LOG_ERROR("ERS → échec création tâche ERS_RX");

    // -----------------------------------------------------------------------
    // Tâche de supervision (timeouts)
    // Priorité 2 → cadence lente, ne doit gêner aucune tâche critique
    // -----------------------------------------------------------------------
    BaseType_t ok2 = xTaskCreate(
        ERS_TaskSupervision,
        "ERS_SUP",
        4096,
        nullptr,
        2,          // ★ priorité corrigée
        nullptr);

    if (ok2 == pdPASS)
        LOG_INFO("ERS → tâche ERS_SUP créée (prio 2)");
    else
        LOG_ERROR("ERS → échec création tâche ERS_SUP");

    LOG_INFO("ERS → démarré");
}
