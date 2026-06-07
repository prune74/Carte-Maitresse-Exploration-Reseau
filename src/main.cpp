/*
main.cpp — Version Discovery 2026

🎯 Rôle
Point d’entrée unique du firmware Discovery 2026.
Il initialise :
- DiscoveryMaster     → WiFi, Web, CAN Discovery, satellites
- DCC2CAN             → Décodage DCC + CAN Booster
- DiscoveryWatchdog   → Supervision heartbeat + STOP d’urgence
*/

#include <Arduino.h>
#include "DiscoveryMaster_main.h"
#include "DCC2CAN_main.h"
#include "DiscoveryWatchdog_main.h"
#include "Debug.h"

// ---------------------------------------------------------------------------
// 🔥 Commande série pour changer le niveau de debug
// ---------------------------------------------------------------------------
void handleSerialDebugCommand()
{
    if (!Serial.available())
        return;

    char c = Serial.read();

    switch (c)
    {
    case '0':
        DEBUG_LEVEL = DEBUG_NONE;
        Serial.println("[DEBUG] Niveau = 0 (NONE)");
        break;

    case '1':
        DEBUG_LEVEL = DEBUG_ERROR;
        Serial.println("[DEBUG] Niveau = 1 (ERROR)");
        break;

    case '2':
        DEBUG_LEVEL = DEBUG_WARN;
        Serial.println("[DEBUG] Niveau = 2 (WARN)");
        break;

    case '3':
        DEBUG_LEVEL = DEBUG_INFO;
        Serial.println("[DEBUG] Niveau = 3 (INFO)");
        break;

    case '4':
        DEBUG_LEVEL = DEBUG_VERBOSE;
        Serial.println("[DEBUG] Niveau = 4 (VERBOSE)");
        break;

    default:
        Serial.println("[DEBUG] Commande inconnue. Utilise 0-4.");
        break;
    }
}

void setup()
{
    Serial.begin(115200);
    delay(200); // Petit délai pour stabiliser l’UART

    LOG_INFO("===============================================");
    LOG_INFO("   D I S C O V E R Y   2 0 2 6   -   M A S T E R");
    LOG_INFO("===============================================");
    LOG_INFO("DebugLevel initial = %u", DEBUG_LEVEL);

    // -----------------------------------------------------------------------
    // INITIALISATION DES MODULES
    // -----------------------------------------------------------------------
    LOG_INFO("Initialisation DiscoveryMaster...");
    DiscoveryMaster_setup();

    LOG_INFO("Initialisation DCC2CAN Booster...");
    Booster_setup();

    LOG_INFO("Initialisation Watchdog Discovery...");
    DiscoveryWatchdog_begin();

    LOG_INFO("Système Discovery 2026 prêt");
}

void loop()
{
    // 🔥 Lecture des commandes série (0-4)
    handleSerialDebugCommand();

    // Boucles logiques des modules
    DiscoveryMaster_loop();
    Booster_loop();

    // FreeRTOS gère les tâches internes (DCC, CAN, Watchdog)
}
