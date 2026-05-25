/*
DiscoveryMaster_main.cpp / .h

🎯 Rôle
Main logique du module SAMain.
Ce fichier encapsule l’ensemble des initialisations et de la boucle principale
du système Discovery : WiFi, Web, bus CAN Discovery (MCP2515) et gestion des
satellites.

Il fournit deux fonctions publiques :
    • DiscoveryMaster_setup() → initialisation complète du module
    • DiscoveryMaster_loop()  → exécution périodique des services Discovery

📌 Fonctionnement
- Initialise les paramètres système (lecture du fichier settings.json).
- Configure le WiFi (mode AP ou STA selon la configuration).
- Démarre le serveur Web et l’API Discovery.
- Initialise le bus CAN Discovery (MCP2515).
- Lance le gestionnaire de satellites (découverte, supervision, mise à jour).
- Exécute en boucle :
    • canService.loop()   → réception et traitement des trames Discovery
    • webHandler.loop()   → gestion des requêtes HTTP/WebSocket
    • satManager.loop()   → supervision des satellites

📌 Particularités
- Ne contient PAS de setup() ni loop() Arduino : ces fonctions sont fournies
  par le main.cpp global du firmware.
- Le module est indépendant du Booster (DCC_CAN-Booster) et du Watchdog.
- Le délai FreeRTOS (vTaskDelay) garantit une exécution non bloquante.
- Les variables globales du module (idMain, satManager, canService, wifi,
  webHandler) sont centralisées ici pour une architecture claire et modulaire.

🔗 Dépendances
- DiscoveryMaster_Settings          → configuration et ID maître
- DiscoveryMaster_Fl_Wifi           → gestion WiFi
- DiscoveryMaster_WebHandler        → interface Web Discovery
- DiscoveryMaster_CanService        → driver CAN MCP2515
- DiscoveryMaster_SatManager        → gestion des satellites Discovery
*/

#include "DiscoveryMaster_main.h"

// ---------------------------------------------------------------------------
// VARIABLES GLOBALES DU MODULE SAMain
// ---------------------------------------------------------------------------
uint16_t idMain = 254;                       // ID maître Discovery
DiscoveryMaster_SatManager satManager;       // Gestionnaire de satellites
DiscoveryMaster_CanService canService(5, 4); // MCP2515 CS=5, INT=4
DiscoveryMaster_Fl_Wifi wifi;                // Wifi
DiscoveryMaster_WebHandler webHandler(&canService);

// ---------------------------------------------------------------------------
// SETUP DU MODULE SAMain
// ---------------------------------------------------------------------------
void DiscoveryMaster_setup()
{
    Serial.printf("\n\nProject :    %s", PROJECT);
    Serial.printf("\nVersion :    %s", VERSION);
    Serial.printf("\nCompiled :   %s - %s\n\n", __DATE__, __TIME__);

    // LED onboard (optionnel)
    pinMode(2, OUTPUT);
    digitalWrite(2, LOW);

    // Settings
    DiscoveryMaster_Settings::begin();
    DiscoveryMaster_Settings::readFile();

    // Wifi + Web
    wifi.start();
    webHandler.init(80);

    // CAN Discovery
    canService.begin();

    // Satellites
    satManager.begin();
}

// ---------------------------------------------------------------------------
// LOOP DU MODULE SAMain
// ---------------------------------------------------------------------------
void DiscoveryMaster_loop()
{
    canService.loop();
    webHandler.loop();
    satManager.loop();

    vTaskDelay(pdMS_TO_TICKS(10));
}
