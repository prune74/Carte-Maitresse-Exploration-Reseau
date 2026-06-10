/*
 * DCC2CAN_main.cpp
 *
 * 🎯 Rôle
 * Point d’entrée du module DCC2CAN.
 *
 * Ce fichier orchestre l’initialisation de tous les sous-modules :
 *   • gestion d’état (BoosterState)
 *   • décodeur DCC (ISR + queue)
 *   • driver CAN Booster (émission uniquement)
 *   • interface CLI série
 *   • création des tâches FreeRTOS
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
 * 🚀 INITIALISATION DU MODULE DCC2CAN
 *
 * Cette fonction configure tous les sous-systèmes nécessaires au traitement
 * du signal DCC et à son émission sur le bus CAN Booster.
 *
 * ⚠️ Aucun traitement lourd ici : uniquement de l’initialisation.
 * ------------------------------------------------------------------------- */
void Booster_setup()
{
    LOG_INFO("DCC2CAN → Initialisation du module Booster");

    /* ---------------------------------------------------------------
     * Gestion d’état
     * ------------------------------------------------------------- */
    BoosterState_init();
    LOG_INFO("BoosterState initialisé");

    /* ---------------------------------------------------------------
     * Décodeur DCC (ISR + queue)
     * ------------------------------------------------------------- */
    DccDecoder_begin();
    LOG_INFO("DCC Decoder initialisé (ISR + queue)");

    /* ---------------------------------------------------------------
     * Driver CAN Booster (TX uniquement)
     * ------------------------------------------------------------- */
    CanBooster_begin();
    LOG_INFO("CAN Booster (CAN0) initialisé");

    /* ---------------------------------------------------------------
     * Interface CLI série
     * ------------------------------------------------------------- */
    Cli_begin();
    LOG_INFO("CLI série initialisé");

    /* ---------------------------------------------------------------
     * Création des tâches FreeRTOS
     * ------------------------------------------------------------- */
    LOG_INFO("Création des tâches FreeRTOS…");

    // Tâche critique : doit tourner en continu, sans delay
    xTaskCreate(taskDcc, "DCC", 4096, NULL, 5, NULL);
    LOG_INFO("Tâche taskDcc créée (prio 5)");

    // Tâche CAN : cadence fixe 2 ms, priorité inférieure
    xTaskCreate(taskCan, "CAN", 4096, NULL, 3, NULL);
    LOG_INFO("Tâche taskCan créée (prio 3)");

    // Supervision : faible priorité, cadence 20 ms
    xTaskCreate(taskSupervision, "SUP", 2048, NULL, 1, NULL);
    LOG_INFO("Tâche taskSupervision créée (prio 1)");

    LOG_INFO("DCC2CAN → Setup terminé");
}

/* ---------------------------------------------------------------------------
 * 🔁 BOUCLE PRINCIPALE
 *
 * La boucle principale est volontairement légère :
 *   • elle exécute la tâche CLI
 *   • elle laisse FreeRTOS gérer les tâches temps réel
 *
 * Aucun traitement bloquant ici.
 * ------------------------------------------------------------------------- */
void Booster_loop()
{
    Cli_task();
    vTaskDelay(pdMS_TO_TICKS(5));   // cadence légère, non critique
}
