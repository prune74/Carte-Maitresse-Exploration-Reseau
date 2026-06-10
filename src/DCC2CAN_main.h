/*
 * DCC2CAN_main.h
 *
 * Interface publique du point d’entrée du module DCC2CAN.
 *
 * Ce fichier déclare les deux fonctions principales :
 *   - Booster_setup() : initialise tous les sous-modules
 *   - Booster_loop()  : boucle légère exécutant le CLI
 *
 * L’ensemble du traitement temps réel (DCC, CAN, supervision) est assuré
 * par les tâches FreeRTOS créées dans Booster_setup().
 */

#pragma once

#include "DCC2CAN_TaskDcc.h"
#include "DCC2CAN_TaskCan.h"
#include "DCC2CAN_Supervision.h"
#include "DCC2CAN_DccDecoder.h"
#include "DCC2CAN_CanBooster.h"
#include "DCC2CAN_Cli.h"

/* ---------------------------------------------------------------------------
   INITIALISATION DU MODULE DCC2CAN
   ---------------------------------------------------------------------------
   Configure :
     - la gestion d’état
     - le décodeur DCC
     - le driver CAN Booster
     - le CLI série
     - les tâches FreeRTOS
--------------------------------------------------------------------------- */
void Booster_setup();

/* ---------------------------------------------------------------------------
   BOUCLE PRINCIPALE
   ---------------------------------------------------------------------------
   Exécute la tâche CLI et laisse FreeRTOS gérer les tâches temps réel.
--------------------------------------------------------------------------- */
void Booster_loop();
