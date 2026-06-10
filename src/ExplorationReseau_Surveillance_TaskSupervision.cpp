/*
 * ExplorationReseau_Surveillance_TaskSupervision.cpp
 *
 * 🎯 Rôle
 * Tâche FreeRTOS chargée d’analyser périodiquement les timeouts des satellites
 * et de déclencher un STOP global en cas de silence prolongé.
 *
 * Cette tâche constitue le cœur du Watchdog ERS.
 * Elle tourne à faible priorité et ne doit jamais perturber les tâches
 * temps réel critiques (DCC2CAN).
 */

#include "ExplorationReseau_Surveillance_Watchdog.h"
#include "Variables.h"
#include "Debug.h"

void ERS_TaskSupervision(void *pv)
{
    (void)pv;

    LOG_INFO("ERS SUP → tâche démarrée (période %u ms)", ERS_SUP_PERIOD_MS);

    for (;;)
    {
        /* -----------------------------------------------------------
         * Analyse des timeouts + STOP global si nécessaire
         * --------------------------------------------------------- */
        ERS_supervise();

        // Log léger : cette tâche n’est pas critique
        LOG_INFO("ERS SUP → supervision exécutée");

        /* -----------------------------------------------------------
         * Cadence fixe (faible fréquence)
         * --------------------------------------------------------- */
        vTaskDelay(pdMS_TO_TICKS(ERS_SUP_PERIOD_MS));
    }
}
