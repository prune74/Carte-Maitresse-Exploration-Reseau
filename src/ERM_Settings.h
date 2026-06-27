#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "ERM_Config.h"

/*
 * 🎯 Rôle
 * Module de gestion des paramètres persistants de la Carte Maîtresse
 * d’Exploration du Réseau (ERM).
 *
 * Les paramètres sont stockés dans :
 *   • /settings.json
 *
 * Ce module fournit :
 *   • le chargement au démarrage
 *   • la sauvegarde à la demande
 *   • des variables statiques accessibles globalement
 *   • des getters simplifiés pour le WiFi
 */

class ERM_Settings
{
public:
    // --- Gestion du fichier settings.json ---
    static void begin();     // Monte SPIFFS
    static void readFile();  // Charge les paramètres
    static void writeFile(); // Sauvegarde les paramètres

    // --- Accès simplifié aux credentials WiFi ---
    static String getWifiSsid();
    static String getWifiPassword();

public:
    // --- Paramètres persistants ---
    static bool WIFI_ON;        // Activation WiFi
    static bool EXPLORATION_ON; // Activation du mode Exploration
    static uint16_t idNode;     // Identifiant maître (1..253)

    static String WIFI_SSID; // SSID WiFi
    static String WIFI_PSW;  // Mot de passe WiFi

    // Profil voie (0 = N, 1 = HO)
    static uint8_t track_profile;

    // Mode test (FakeDCC + CAN loopback)
    static bool MODE_TEST;

    // Empêche l’instanciation
    ERM_Settings() = delete;
};
