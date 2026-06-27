#pragma once
#include <Arduino.h>
#include "ERS_Watchdog.h"

/*
 * ERS_main.h
 *
 * 🎯 Rôle
 * Point d’entrée du module de Surveillance ERS.
 *
 * Ce module initialise :
 *   • la tâche de réception des heartbeats (ERS_TaskRx)
 *   • la tâche de supervision des timeouts (ERS_TaskSupervision)
 *
 * Il constitue la couche Watchdog du système ERM.
 *
 * 📌 Architecture
 * - ERS_begin() est l’unique fonction publique d’initialisation.
 * - Les tâches FreeRTOS sont créées ici.
 * - La logique interne (supervision, heartbeat, STOP global)
 *   est gérée dans ERS_Watchdog.cpp.
 */

// Initialisation complète du module ERS (tâches + structures internes)
void ERS_begin();
