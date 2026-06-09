#pragma once
#include <Arduino.h>
#include "ExplorationReseau_Maitre_Config.h"

/*
 * 🎯 Rôle
 * Représentation minimale d’un satellite dans la Carte Maîtresse
 * d’Exploration du Réseau (ERM).
 *
 * Ce struct est volontairement simple :
 *   • aucune logique interne
 *   • aucune allocation dynamique
 *   • utilisé dans un tableau fixe géré par ERM_SatManager
 *
 * Il sert uniquement de conteneur de données pour :
 *   • l’identifiant du satellite
 *   • son adresse IP éventuelle
 *   • son état (online/offline)
 *   • son dernier heartbeat
 *   • des informations optionnelles (type, version)
 */

struct ERM_Satellite
{
    uint16_t id = NO_ID;          // Identifiant du satellite (1..253)

    uint8_t ip[4] = {0,0,0,0};    // Optionnel : adresse IP du satellite

    uint32_t lastSeen = 0;        // Timestamp du dernier heartbeat reçu
    bool online = false;          // État logique (mis à jour par ERM_SatManager)

    uint8_t type = 0;             // Optionnel : type de satellite
    uint8_t version = 0;          // Optionnel : version firmware
};
