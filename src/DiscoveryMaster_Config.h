/*
DiscoveryMaster_Config.h

🎯 Rôle
Fichier de configuration global du module SAMain.
Il regroupe toutes les constantes, paramètres système, définitions de projet
et options de compilation nécessaires au fonctionnement du réseau Discovery.

📌 Contenu
- Informations projet :
    • PROJECT  → nom du firmware
    • VERSION  → version logicielle
- Constantes système :
    • NO_ID, NO_PIN → valeurs par défaut
    • NB_SAT        → nombre maximum de satellites Discovery
    • NB_LOCOS      → nombre maximum de locomotives gérées
- Options de debug :
    • DEBUG
    • alias debug → Serial
- Configuration WiFi :
    • Mode AP ou STA selon CONFIG
    • SSID / mot de passe
- Dépendances :
    • ACAN2515 pour le bus CAN Discovery
    • Arduino pour les types et macros de base

📌 Particularités
- Ce fichier est inclus dans la majorité des modules SAMain (WiFi, WebHandler,
  CanService, Settings, etc.).
- Les paramètres WiFi peuvent être modifiés ici pour adapter le comportement
  du maître Discovery.
- Les constantes NB_SAT et NB_LOCOS définissent les limites du système et
  influencent la gestion mémoire du projet.

*/

#pragma once
#include <Arduino.h>
#include <ACAN2515.h>

#define PROJECT "Discovery Master Board"
#define VERSION "v0.1"

#define DEBUG

#define NO_ID 255
#define NO_PIN 255

#define NB_SAT 30
#define NB_LOCOS 7

/* ----- Debug   -------------------*/
#define DEBUG
#define debug Serial

/* ----- Wifi --------------------*/

#define CONFIG 1 // Selection du mode

#if CONFIG == 0 // WiFi en mode point d’accès
#define WIFI_AP_MODE
#define WIFI_SSID "digital"
#define WIFI_PSW "digital" // Password facultatif

#elif CONFIG == 1
// Les identifiants WiFi sont désormais chargés depuis settings.json
// Voir DiscoveryMaster_Settings pour la configuration du WiFi en mode STA
#endif

/* ---------------------------------------------------------------------------
   IDs CAN Discovery 2026
   --------------------------------------------------------------------------- */

// Heartbeat envoyé par les satellites
#define DISCOVERY_CAN_ID_HEARTBEAT 0x200

// STOP d’urgence envoyé par le Watchdog
#define DISCOVERY_CAN_ID_EMERGENCY_STOP 0x201
