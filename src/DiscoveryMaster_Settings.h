#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "DiscoveryMaster_Config.h"

class DiscoveryMaster_Settings
{
public:
    static bool WIFI_ON;
    static bool DISCOVERY_ON;
    static uint16_t idNode;
    static String WIFI_SSID;
    static String WIFI_PSW;

    DiscoveryMaster_Settings() = delete;

    static void begin();
    static void readFile();
    static void writeFile();
    static String getWifiSsid();
    static String getWifiPassword();
};
