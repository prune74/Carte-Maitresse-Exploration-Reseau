/*
DCC2CAN_main.cpp / .h

🎯 Rôle
Point d’entrée du module DCC2CAN. Ce fichier initialise les sous-modules
(DCC, CAN Booster, CLI) et crée les tâches FreeRTOS nécessaires au traitement
temps réel du signal DCC et à son émission sur le bus CAN Booster.

📌 Fonctionnement
- Booster_setup() :
    • Initialise la gestion d’état (BoosterState)
    • Initialise le décodeur DCC (ISR + file d’événements)
    • Initialise le driver CAN Booster (émission uniquement)
    • Initialise le CLI série
    • Initialise la pile CAN via CanUniversal (CAN0 = ESP32 interne)
    • Crée les tâches FreeRTOS :
        - taskDcc        → traitement des événements DCC
        - taskCan        → envoi des trames CAN Booster
        - taskSupervision→ surveillance du signal DCC (failsafe)

- Booster_loop() :
    • Exécute la tâche CLI (lecture des commandes série)
    • Boucle légère et non bloquante (FreeRTOS gère les tâches temps réel)

📌 Particularités
- Le bus CAN Booster est unidirectionnel pour DCC2CAN : émission uniquement.
- Aucune réception CAN n’est utilisée ni prévue dans ce module.
- Le découpage en modules séparés (DccDecoder, TaskDcc, TaskCan, Supervision,
  State, CLI) garantit une architecture claire et maintenable.
- Le main reste volontairement minimal : aucune logique métier ici.
*/

#include "DCC2CAN_main.h"
#include "CanInit.h"
#include "CanBus.h"
#include "CanMsg.h"
#include "DCC2CAN_FakeDcc.h"
#include "Debug.h"   // 🔥 Ajout du système de logs


void Booster_setup()
{
    LOG_INFO("DCC2CAN → Initialisation du module Booster");

    // -------------------------------------------------------------------------
    // Gestion d’état
    // -------------------------------------------------------------------------
    BoosterState_init();
    LOG_VERBOSE("BoosterState initialisé");

    // -------------------------------------------------------------------------
    // Décodeur DCC (ISR + file d’événements)
    // -------------------------------------------------------------------------
    DccDecoder_begin();
    LOG_INFO("DCC Decoder initialisé (ISR + queue)");

    // -------------------------------------------------------------------------
    // Driver CAN Booster (TX only)
    // -------------------------------------------------------------------------
    CanBooster_begin();
    LOG_INFO("CAN Booster (CAN0) initialisé");

    // -------------------------------------------------------------------------
    // CLI série
    // -------------------------------------------------------------------------
    Cli_begin();
    LOG_INFO("CLI série initialisé");

    // -------------------------------------------------------------------------
    // Tâches FreeRTOS
    // -------------------------------------------------------------------------
    LOG_INFO("Création des tâches FreeRTOS…");

    xTaskCreate(taskDcc, "DCC", 4096, NULL, 3, NULL);
    LOG_VERBOSE("Tâche taskDcc créée");

    xTaskCreate(taskCan, "CAN", 4096, NULL, 3, NULL);
    LOG_VERBOSE("Tâche taskCan créée");

    xTaskCreate(taskSupervision, "SUP", 2048, NULL, 1, NULL);
    LOG_VERBOSE("Tâche taskSupervision créée");

    LOG_INFO("DCC2CAN → Setup terminé");
}

void Booster_loop()
{
    Cli_task();
    vTaskDelay(pdMS_TO_TICKS(5));
}
