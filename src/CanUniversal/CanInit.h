#pragma once

#include "CanBus.h"
#include <ACAN_ESP32.h>
#include <ACAN2515.h>

/**
 * ============================================================================
 *  CanUniversal_begin()
 * ============================================================================
 *  Initialise tous les bus CAN définis dans CanConfig.h.
 *
 *  Cette fonction :
 *    - configure ACAN_ESP32 (si CAN0 utilise l’ESP32 interne)
 *    - configure ACAN2515  (si CAN1, CAN2… utilisent un MCP2515)
 *    - instancie les objets CanBus dans le tableau global CAN[]
 *    - renvoie true si tout est OK
 *
 *  Objectif :
 *    → garder un setup() propre
 *    → rendre la bibliothèque totalement générique
 *    → ne rien imposer à l’application (noms, rôles, vitesses…)
 *
 *  L’application décide :
 *    - combien de bus CAN existent
 *    - leur rôle (System, Booster, Gateway…)
 *    - leur vitesse
 *    - leurs broches
 * ============================================================================
 */

// Initialise tous les bus CAN
bool CanUniversal_begin();

// -----------------------------------------------------------------------------
// Tableau global de bus CAN
// -----------------------------------------------------------------------------
// CAN[0], CAN[1], CAN[2]…
// L’application décide comment les nommer :
//   #define CAN_SYSTEM  CAN[0]
//   #define CAN_BOOSTER CAN[1]
// -----------------------------------------------------------------------------
extern CanBus CAN[];

// -----------------------------------------------------------------------------
// Drivers ACAN internes à la bibliothèque
// -----------------------------------------------------------------------------
// ACAN_ESP32 n’a pas besoin d’instance (driver statique ACAN_ESP32::can)
// ACAN2515 doit être instancié ici pour être accessible à CanInit.cpp
// -----------------------------------------------------------------------------
extern ACAN2515 can2515;   // utilisé si un bus CAN utilise un MCP2515
