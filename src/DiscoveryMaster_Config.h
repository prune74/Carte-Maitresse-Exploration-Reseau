/*
DiscoveryMaster_Config.h

🎯 Rôle
Fichier de configuration global du module DiscoveryMaster.
Il regroupe les constantes système, les définitions du protocole Discovery
et les options de compilation nécessaires au fonctionnement du réseau Discovery 2026.

📌 Contenu
- Informations projet :
    • PROJECT  → nom du firmware
    • VERSION  → version logicielle

- Constantes système :
    • NO_ID, NO_PIN → valeurs par défaut
    • NB_SAT        → nombre maximum de satellites Discovery

- Options de debug :
    • DEBUG
    • alias debug → Serial

- Configuration WiFi :
    • Activation du mode AP si WIFI_AP_MODE est défini
    • Sinon, fonctionnement en mode STA (client WiFi)
    • Les identifiants WiFi (SSID / mot de passe) sont chargés depuis settings.json

- Protocole Discovery :
    • IDs CAN
    • commandes
    • masques et filtres

📌 Particularités
- Ce fichier est inclus dans la majorité des modules DiscoveryMaster
  (WiFi, WebHandler, CanService, Settings, SatManager, etc.).
- Les paramètres dynamiques (WiFi, Discovery, credentials) sont désormais
  stockés dans settings.json et gérés par DiscoveryMaster_Settings.
*/

#pragma once
#include <Arduino.h>
#include <ACAN2515.h>

/* ----- Informations projet ----- */
#define PROJECT "Discovery Master Board"
#define VERSION "v0.1"

/* ----- Debug ----- */
#define DEBUG
#define debug Serial

/* ----- Constantes système ----- */
#define NO_ID 255
#define NO_PIN 255
#define NB_SAT 30

/* ----- WiFi : Mode AP (optionnel) -----
   Décommentez la ligne ci-dessous pour forcer le mode AP.
   Si WIFI_AP_MODE n'est PAS défini :
       → la carte démarre en mode STA (client WiFi)
       → SSID / mot de passe chargés depuis settings.json
*/
//#define WIFI_AP_MODE

/* ---------------------------------------------------------------------------
   Protocole Discovery 2026 : IDs CAN
   --------------------------------------------------------------------------- */

// Heartbeat envoyé par les satellites
#define DISCOVERY_CAN_ID_HEARTBEAT       0x200

// STOP d’urgence envoyé par le Watchdog
#define DISCOVERY_CAN_ID_EMERGENCY_STOP  0x201
