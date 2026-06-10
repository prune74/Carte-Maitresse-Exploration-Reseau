/*
 * DCC2CAN_main.cpp
 *
 * Point d’entrée du module DCC2CAN.
 *
 * Ce fichier orchestre l’initialisation de tous les sous-modules :
 *   - gestion d’état (BoosterState)
 *   - décodeur DCC (ISR + queue)
 *   - driver CAN Booster (émission uniquement)
 *   - interface CLI série
 *   - création des tâches FreeRTOS
 *
 * Le main reste volontairement minimal : aucune logique métier ici.
 * FreeRTOS prend ensuite le relais pour exécuter les tâches temps réel.
 */

#include "DCC2CAN_main.h"
#include "DCC2CAN_State.h"
#include "DCC2CAN_DccDecoder.h"
#include "DCC2CAN_CanBooster.h"
#include "DCC2CAN_TaskDcc.h"
#include "DCC2CAN_TaskCan.h"
#include "DCC2CAN_Supervision.h"
#include "DCC2CAN_Cli.h"

#include "CanInit.h"
#include "CanBus.h"
#include "CanMsg.h"
#include "Debug.h"

/* ---------------------------------------------------------------------------
   INITIALISATION DU MODULE DCC2CAN
   ---------------------------------------------------------------------------
   Cette fonction configure tous les sous-systèmes nécessaires au traitement
   du signal DCC et à son émission sur le bus CAN Booster.
--------------------------------------------------------------------------- */
void Booster_setup()
{
    LOG_INFO("DCC2CAN → Initialisation du module Booster");

    /* ---------------------------------------------------------------
       GESTION D’ÉTAT
       --------------------------------------------------------------- */
    BoosterState_init();
    LOG_VERBOSE("BoosterState initialisé");

    /* ---------------------------------------------------------------
       DÉCODEUR DCC (ISR + queue)
       --------------------------------------------------------------- */
    DccDecoder_begin();
    LOG_INFO("DCC Decoder initialisé (ISR + queue)");

    /* ---------------------------------------------------------------
       DRIVER CAN BOOSTER (TX uniquement)
       --------------------------------------------------------------- */
    CanBooster_begin();
    LOG_INFO("CAN Booster (CAN0) initialisé");

    /* ---------------------------------------------------------------
       INTERFACE CLI SÉRIE
       --------------------------------------------------------------- */
    Cli_begin();
    LOG_INFO("CLI série initialisé");

    /* ---------------------------------------------------------------
       CRÉATION DES TÂCHES FREERTOS
       --------------------------------------------------------------- */
    LOG_INFO("Création des tâches FreeRTOS…");

    xTaskCreate(taskDcc, "DCC", 4096, NULL, 3, NULL);
    LOG_VERBOSE("Tâche taskDcc créée");

    xTaskCreate(taskCan, "CAN", 4096, NULL, 3, NULL);
    LOG_VERBOSE("Tâche taskCan créée");

    xTaskCreate(taskSupervision, "SUP", 2048, NULL, 1, NULL);
    LOG_VERBOSE("Tâche taskSupervision créée");

    LOG_INFO("DCC2CAN → Setup terminé");
}

/* ---------------------------------------------------------------------------
   BOUCLE PRINCIPALE
   ---------------------------------------------------------------------------
   La boucle principale est volontairement légère :
     - elle exécute la tâche CLI
     - elle laisse FreeRTOS gérer les tâches temps réel
--------------------------------------------------------------------------- */
void Booster_loop()
{
    Cli_task();
    vTaskDelay(pdMS_TO_TICKS(5));
}
