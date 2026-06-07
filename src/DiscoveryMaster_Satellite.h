#pragma once
#include <Arduino.h>
#include "DiscoveryMaster_Config.h"

/*
 * 🎯 Rôle
 * Représentation d’un satellite Discovery dans le Master.
 * Ce struct est volontairement minimal et optimisé :
 * - aucune logique
 * - aucune allocation dynamique
 * - utilisé en tableau fixe dans SatManager
 */

struct DiscoveryMaster_Satellite
{
    uint16_t id = NO_ID;          // Identifiant Discovery (1..253)

    uint8_t ip[4] = {0,0,0,0};    // Optionnel : IP du satellite (si utile)

    uint32_t lastSeen = 0;        // Timestamp du dernier heartbeat
    bool online = false;          // État logique (mis à jour par SatManager)

    uint8_t type = 0;             // Optionnel : type de satellite
    uint8_t version = 0;          // Optionnel : version firmware
};
