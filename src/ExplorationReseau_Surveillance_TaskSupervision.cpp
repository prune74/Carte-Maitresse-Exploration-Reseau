/*
 * ExplorationReseau_Surveillance_TaskSupervision.cpp
 *
 * 🎯 Rôle
 * Tâche FreeRTOS chargée d’analyser périodiquement les timeouts des satellites
 * et de déclencher un STOP global en cas de silence prolongé.
 *
 * Cette tâche constitue le cœur du Watchdog ERS.
 */

#include "ExplorationReseau_Surveillance_Watchdog.h"
#include "Debug.h"

void ERS_TaskSupervision(void *pv)
{
    LOG_INFO("ERS SUP → tâche démarrée (période %u ms)", ERS_PERIOD_MS);

    for (;;)
    {
        // Analyse des timeouts + STOP global si nécessaire
        ERS_supervise();
        LOG_VERBOSE("ERS SUP → supervision exécutée");

        // Période de supervision
        vTaskDelay(pdMS_TO_TICKS(ERS_PERIOD_MS));
    }
}
