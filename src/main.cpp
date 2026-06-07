/*
main.cpp

🎯 Rôle
Point d’entrée unique du firmware Discovery 2026. Ce fichier initialise les
trois sous-systèmes principaux :
- DiscoveryMaster     → Réseau Discovery : WiFi, Web, CAN MCP2515, satellites
- DCC2CAN             → Décodage DCC + émission CAN Booster
- DiscoveryWatchdog   → Supervision heartbeat + STOP d’urgence

Il orchestre l’initialisation globale et délègue ensuite l’exécution aux
boucles logiques de chaque module, tandis que FreeRTOS gère les tâches
temps réel internes.

📌 Fonctionnement
- setup() :
    • Initialise la liaison série
    • Lance DiscoveryMaster_setup()     → WiFi, Web, CAN Discovery, satellites
    • Lance Booster_setup()             → DCC, CAN Booster (TX only)
    • Lance DiscoveryWatchdog_begin()   → Tâches Watchdog (Rx + supervision)

- loop() :
    • Exécute DiscoveryMaster_loop()    → Web, CAN Discovery, gestion satellites
    • Exécute Booster_loop()            → CLI + supervision DCC2CAN
    • FreeRTOS gère les tâches temps réel (DCC, CAN TX, Watchdog)

📌 Particularités
- Architecture modulaire : chaque sous-système possède son propre “main logique”
  (DiscoveryMaster_main.cpp, DCC2CAN_main.cpp, DiscoveryWatchdog_main.cpp).
- Les deux bus CAN sont totalement indépendants :
    • CAN Discovery → MCP2515 (SPI)
    • CAN Booster   → CAN interne ESP32 (émission uniquement pour DCC2CAN)
- Le main.cpp reste volontairement minimal : aucune logique métier ici.

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
    
    for (int i = 0; i < 20; i++)
    vTaskDelay(pdMS_TO_TICKS(10));

    DiscoveryMaster_setup();
    Booster_setup();
    DiscoveryWatchdog_begin();
}

void loop()
{
    DiscoveryMaster_loop();
    Booster_loop();
}

/*
#include <ACAN2515.h>
#include <SPI.h>

ACAN2515 can (13, SPI, 14); // CS=13, INT=14

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("Test MCP2515...");
  SPI.begin(18, 19, 23, 13);
  ACAN2515Settings settings(8000000, 250000, 50);
  uint32_t err = can.begin(settings, [](){});
  Serial.printf("Erreur init: 0x%X\n", err);
}

void loop() {}
*/
