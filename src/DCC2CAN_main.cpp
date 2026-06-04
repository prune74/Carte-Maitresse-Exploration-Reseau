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

🔗 Dépendances
- DCC2CAN_DccDecoder   → décodage DCC
- DCC2CAN_CanBooster   → émission CAN Booster
- DCC2CAN_Cli          → interface série
- DCC2CAN_TaskDcc      → tâche DCC
- DCC2CAN_TaskCan      → tâche CAN (TX)
- DCC2CAN_Supervision  → failsafe / supervision
- FreeRTOS             → multitâche temps réel
*/

#include "DCC2CAN_main.h"
#include "CanUniversal/CanInit.h"
#include "CanUniversal/CanBus.h"
#include "CanUniversal/CanMsg.h"

void Booster_setup()
{
    // -------------------------------------------------------------------------
    // Gestion d’état
    // -------------------------------------------------------------------------
    BoosterState_init();

    // -------------------------------------------------------------------------
    // Décodeur DCC (ISR + file d’événements)
    // -------------------------------------------------------------------------
    DccDecoder_begin();

    // -------------------------------------------------------------------------
    // Initialisation CAN (CAN0 via CanUniversal)
    // -------------------------------------------------------------------------
    CanUniversal_begin();

    // -------------------------------------------------------------------------
    // Driver CAN Booster (TX only)
    // -------------------------------------------------------------------------
    CanBooster_begin();

    // -------------------------------------------------------------------------
    // CLI série
    // -------------------------------------------------------------------------
    Cli_begin();

    // -------------------------------------------------------------------------
    // Tâches FreeRTOS
    // -------------------------------------------------------------------------
    xTaskCreate(taskDcc,        "DCC", 4096, NULL, 3, NULL);
    xTaskCreate(taskCan,        "CAN", 4096, NULL, 3, NULL);
    xTaskCreate(taskSupervision,"SUP", 2048, NULL, 1, NULL);
}

void Booster_loop()
{
    Cli_task();
    vTaskDelay(pdMS_TO_TICKS(5));
}
