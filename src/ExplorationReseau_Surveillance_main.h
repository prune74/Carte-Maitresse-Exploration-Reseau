#pragma once
#include <Arduino.h>
#include "ExplorationReseau_Surveillance_Watchdog.h"

/*
 * ExplorationReseau_Surveillance_main.h
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

// Initialisation complète du module ERS
void ERS_begin();
