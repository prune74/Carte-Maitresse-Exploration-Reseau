/*
main.cpp

🎯 Rôle
Point d’entrée unique du firmware combinant les trois modules principaux :
- SAMain            → Réseau Discovery : WiFi, Web, CAN MCP2515, satellites
- DCC_CAN-Booster   → Décodage DCC, RailCom, CAN Booster, failsafe
- SAWatchdog        → Supervision heartbeat + STOP d’urgence Discovery 2026

Ce fichier orchestre l’initialisation et l’exécution de l’ensemble du système,
tout en laissant FreeRTOS gérer les tâches internes de chaque module.

📌 Fonctionnement
- setup() :
    • Initialise la liaison série
    • Lance DiscoveryMaster_setup()          → WiFi, Web, CAN Discovery, satellites
    • Lance Booster_setup()         → DCC, RailCom, CAN Booster
    • Lance DiscoveryWatchdog_begin()      → Tâches Watchdog (Rx + supervision)

- loop() :
    • Exécute DiscoveryMaster_loop()         → Web, CAN Discovery, gestion satellites
    • Exécute Booster_loop()        → CLI + supervision Booster
    • Laisse FreeRTOS gérer les tâches DCC, CAN et Watchdog

📌 Particularités
- Architecture modulaire : chaque module possède son propre “main logique”
  (DiscoveryMaster_main.cpp, DCC2CAN_main.cpp, DiscoveryWatchdog_main.cpp).
- Les deux bus CAN sont totalement indépendants :
    • CAN Discovery → MCP2515 (SPI)
    • CAN Booster   → CAN interne ESP32
- Le main.cpp reste minimal et propre : aucune logique métier.

🔗 Dépendances
- DiscoveryMaster_main.h
- DCC2CAN_main.h
- DiscoveryWatchdog_main.h
- FreeRTOS (tâches créées dans Booster_setup et DiscoveryWatchdog_begin)
*/

#include <Arduino.h>
#include "DiscoveryMaster_main.h"
#include "DCC2CAN_main.h"
#include "DiscoveryWatchdog_main.h"

void setup()
{
    Serial.begin(115200);
    delay(200);

    DiscoveryMaster_setup();
    Booster_setup();
    DiscoveryWatchdog_begin();
}

void loop()
{
    DiscoveryMaster_loop();
    Booster_loop();
}
