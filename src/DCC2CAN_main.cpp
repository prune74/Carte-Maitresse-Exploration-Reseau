/*
DCC2CAN_main.cpp / .h

🎯 Rôle
Main logique du module DCC_CAN-Booster.
Ce fichier orchestre l’initialisation du Booster et la création des tâches
FreeRTOS responsables du traitement temps réel : DCC, CAN Booster, supervision
et CLI. Il constitue le point d’entrée du sous-système Booster.

📌 Fonctionnement
- Booster_setup() :
    • Initialise le décodeur DCC (ISR + file d’événements)
    • Initialise le driver CAN Booster (CAN interne ESP32)
    • Initialise le CLI série
    • Crée les tâches FreeRTOS :
        - taskDcc        → traitement des événements DCC
        - taskCan        → envoi des trames CAN Booster
        - taskCanRx      → réception CAN Booster (optionnel)
        - taskSupervision→ surveillance et failsafe

- Booster_loop() :
    • Exécute la tâche CLI (lecture des commandes série)
    • Laisse FreeRTOS gérer les tâches temps réel
    • Boucle légère et non bloquante

📌 Particularités
- Le module est totalement indépendant du réseau Discovery (MCP2515).
- Le découpage en fichiers séparés (TaskDcc, TaskCan, TaskCanRx, Supervision,
  State) garantit une architecture claire, modulaire et maintenable.
- Le traitement DCC est interrupt-driven, tandis que le CAN et la supervision
  fonctionnent en tâches FreeRTOS.
- Le main reste volontairement minimal : aucune logique métier ici.

🔗 Dépendances
- DCC2CAN_DccDecoder   → décodage DCC
- DCC2CAN_CanBooster   → driver CAN Booster
- DCC2CAN_Cli          → interface série
- DCC2CAN_TaskDcc      → tâche DCC
- DCC2CAN_TaskCan      → tâche CAN
- DCC2CAN_TaskCanRx    → tâche CAN RX
- DCC2CAN_Supervision  → failsafe / supervision
- FreeRTOS                     → multitâche temps réel
*/

#include "DCC2CAN_main.h"

void Booster_setup()
{
    // Initialize state management
    BoosterState_init();

    // DCC decoder
    DccDecoder_begin();

    // CAN Booster
    CanBooster_begin();

    // CLI
    Cli_begin();

    // Tâches FreeRTOS
    xTaskCreate(taskDcc, "DCC", 4096, NULL, 3, NULL);
    xTaskCreate(taskCan, "CAN", 4096, NULL, 3, NULL);
    xTaskCreate(taskCanRx, "CANRX", 4096, NULL, 2, NULL);
    xTaskCreate(taskSupervision, "SUP", 2048, NULL, 1, NULL);
}

void Booster_loop()
{
    Cli_task();
    vTaskDelay(pdMS_TO_TICKS(5));
}
