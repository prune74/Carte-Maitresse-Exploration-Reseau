#pragma once
#include <Arduino.h>
#include "ERM_Config.h"

/*
 * 🎯 Rôle
 * Représentation minimale d’un Canton Controller dans la Carte Maîtresse
 * d’Exploration du Réseau (ERM).
 *
 * Ce struct est volontairement simple :
 *   • aucune logique interne
 *   • aucune allocation dynamique
 *   • utilisé dans un tableau fixe géré par ERM_CC_Manager
 *
 * Il sert uniquement de conteneur de données pour :
 *   • l’identifiant du Canton Controller
 *   • son adresse IP éventuelle
 *   • son état (online/offline)
 *   • son dernier heartbeat
 *   • des informations optionnelles (type, version)
 */

struct ERM_CantonController
{
    uint16_t id = NO_ID; // Identifiant du Canton Controller (1..253)

    uint8_t ip[4] = {0, 0, 0, 0}; // Optionnel : adresse IP du Canton Controller

    uint32_t lastSeen = 0; // Timestamp du dernier heartbeat reçu
    bool online = false;   // État logique (mis à jour par ERM_CC_Manager)

    uint8_t type = 0;    // Optionnel : type de Canton Controller
    uint8_t version = 0; // Optionnel : version firmware
};
