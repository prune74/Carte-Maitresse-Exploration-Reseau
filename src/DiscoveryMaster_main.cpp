/*
 * DiscoveryMaster_main.cpp / .h
 *
 * 🎯 Rôle
 * Main logique du module SAMain.
 * Ce fichier encapsule l’ensemble des initialisations et de la boucle principale
 * du système Discovery : WiFi, Web, bus CAN Discovery (MCP2515) et gestion des
 * satellites.
 */

#include "DiscoveryMaster_main.h"
#include "CanInit.h"
#include "CAN_Config.h"
#include "Debug.h"

extern MasterConfig MASTER_CAN_CONFIG;

// ---------------------------------------------------------------------------
// VARIABLES GLOBALES DU MODULE SAMain
// ---------------------------------------------------------------------------
uint16_t idMain = 254;                              // ID maître Discovery
DiscoveryMaster_SatManager satManager;              // Gestionnaire de satellites
DiscoveryMaster_CanService canService;              // Service CAN Discovery
DiscoveryMaster_Fl_Wifi wifi;                       // Gestion WiFi
DiscoveryMaster_WebHandler webHandler(&canService); // Interface Web Discovery

// ---------------------------------------------------------------------------
// SETUP DU MODULE SAMain
// ---------------------------------------------------------------------------
void DiscoveryMaster_setup()
{
    LOG_INFO("===============================================");
    LOG_INFO("Project :    %s", PROJECT);
    LOG_INFO("Version :    %s", VERSION);
    LOG_INFO("Compiled :   %s - %s", __DATE__, __TIME__);
    LOG_INFO("===============================================");

    // LED onboard (optionnel)
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);

    // -----------------------------------------------------------------------
    // SETTINGS
    // -----------------------------------------------------------------------
    LOG_INFO("Chargement des paramètres (settings.json)...");
    DiscoveryMaster_Settings::begin();
    DiscoveryMaster_Settings::readFile();

    // -----------------------------------------------------------------------
    // 🟦 INITIALISATION DES 2 BUS CAN (Booster + Discovery)
    // -----------------------------------------------------------------------
    LOG_INFO("Initialisation des bus CAN (CAN0 + CAN1)...");
    CanInit::begin(MASTER_CAN_CONFIG);
    
    vTaskDelay(pdMS_TO_TICKS(5)); // Laisse TWAI sortir du RESET
    CANMessage test;
    test.id = 0x123;
    test.len = 2;
    test.data[0] = 0xAA;
    test.data[1] = 0xBB;

    bool ok = ACAN_ESP32::can.tryToSend(test);
    Serial.printf("[TEST DIRECT TWAI] tryToSend = %d\n", ok);

    CanMsg testMsg((uint16_t)0x123, {0xAA, 0xBB});
    bool okBus = CanBus::bus(0).send(testMsg);
    LOG_INFO("TEST CanUniversal → CanBus::bus(0).send() = %d", okBus);

    // -----------------------------------------------------------------------
    // WIFI + WEB
    // -----------------------------------------------------------------------
    LOG_INFO("Initialisation WiFi...");
    wifi.start();

    LOG_INFO("Initialisation WebHandler (port 80)...");
    webHandler.init(80);

    // -----------------------------------------------------------------------
    // CAN Discovery
    // -----------------------------------------------------------------------
    LOG_INFO("Initialisation du service CAN Discovery...");
    canService.begin();

    // -----------------------------------------------------------------------
    // SATELLITES
    // -----------------------------------------------------------------------
    LOG_INFO("Initialisation du gestionnaire de satellites...");
    satManager.begin();

    LOG_INFO("DiscoveryMaster_setup() terminé");
}

// ---------------------------------------------------------------------------
// LOOP DU MODULE SAMain
// ---------------------------------------------------------------------------
void DiscoveryMaster_loop()
{
    canService.loop();
    webHandler.loop();
    satManager.loop();

    vTaskDelay(pdMS_TO_TICKS(10)); // Exécution non bloquante FreeRTOS
}
