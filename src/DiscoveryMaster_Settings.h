#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "DiscoveryMaster_Config.h"

class DiscoveryMaster_Settings
{
public:
    // --- Paramètres persistants ---
    static bool WIFI_ON;
    static bool DISCOVERY_ON;
    static uint16_t idNode;
    static String WIFI_SSID;
    static String WIFI_PSW;

    // 🔥 Profil voie (0 = N, 1 = HO)
    static uint8_t track_profile;

    // 🔥 Mode test (FakeDCC + CAN loopback)
    static bool MODE_TEST;

    DiscoveryMaster_Settings() = delete;

    // --- Gestion du fichier settings.json ---
    static void begin();
    static void readFile();
    static void writeFile();

    // --- Accès simplifié aux credentials WiFi ---
    static String getWifiSsid();
    static String getWifiPassword();
};
