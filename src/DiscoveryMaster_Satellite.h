#pragma once
#include <Arduino.h>
#include "DiscoveryMaster_Config.h"

struct DiscoveryMaster_Satellite
{
    uint16_t id = NO_ID;

    uint8_t ip[4] = {0,0,0,0};   // optionnel si tu veux garder l’IP

    uint32_t lastSeen = 0;       // mis à jour par le heartbeat
    bool online = false;         // mis à jour par SatManager

    uint8_t type = 0;            // optionnel (booster, gateway, sensor…)
    uint8_t version = 0;         // optionnel (firmware)
};
