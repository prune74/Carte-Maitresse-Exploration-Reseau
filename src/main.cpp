/*
 * main.cpp — Version ERM (Exploration du Réseau Maître)
 *
 * 🎯 Rôle
 * Point d’entrée unique du firmware ERM.
 *
 * Initialise :
 *   • ERM_Maitre      → WiFi, Web, CAN, satellites
 *   • DCC2CAN Booster → Décodage DCC + émission CAN
 *   • ERS             → Surveillance (heartbeats + STOP d’urgence)
 */

#include <Arduino.h>
#include "ExplorationReseau_Maitre_main.h"
#include "DCC2CAN_main.h"
#include "ExplorationReseau_Surveillance_main.h"
#include "Debug.h"

void setup()
{
    Serial.begin(115200);
    delay(200); // Stabilisation UART

    LOG_INFO("===============================================");
    LOG_INFO("   E X P L O R A T I O N   R E S E A U   -   M A I T R E");
    LOG_INFO("===============================================");
    LOG_INFO("DebugLevel initial = %u", DEBUG_LEVEL);

    // -----------------------------------------------------------------------
    // INITIALISATION DES MODULES
    // -----------------------------------------------------------------------
    LOG_INFO("Initialisation ERM_Maitre...");
    ERM_setup();

    LOG_INFO("Initialisation DCC2CAN Booster...");
    Booster_setup();

    LOG_INFO("Initialisation ERS (Surveillance)...");
    ERS_begin();

    LOG_INFO("Système ERM prêt");
}

void loop()
{
    // 🔥 Commande série pour changer le niveau de debug (0-4)
    Debug_handleSerialCommand();

    // Boucles logiques des modules
    ERM_loop();
    Booster_loop();

    // FreeRTOS gère les tâches internes (CAN, DCC, ERS)
}
