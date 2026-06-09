#pragma once
#include <Arduino.h>
#include "ExplorationReseau_Maitre_Config.h"
#include "ExplorationReseau_Maitre_Satellite.h"

/*
 * 🎯 Rôle
 * Gestionnaire des satellites du réseau d’exploration.
 *
 * Ce module maintient :
 *   • un tableau fixe de ERM_Satellite
 *   • la découverte automatique
 *   • les heartbeats
 *   • l’état online/offline
 *   • l’interface de diagnostic pour ERS
 *
 * API modernisée :
 *   • begin() / end() → itération style STL
 */

class ERM_SatManager
{
public:
    ERM_SatManager();

    void begin();
    void loop();

    void addOrUpdate(uint16_t idSat);
    void updateHeartbeat(uint16_t idSat);
    void checkTimeouts(uint32_t timeoutMs);

    ERM_Satellite *getById(uint16_t idSat);
    uint8_t count() const;

    bool hasOfflineSatellite(uint16_t &offlineId) const;

    // -----------------------------------------------------------------------
    // 🆕 API moderne : itération style STL
    // -----------------------------------------------------------------------
    const ERM_Satellite* satBegin() const { return _sats; }
    const ERM_Satellite* satEnd()   const { return _sats + NB_SAT; }

    // Version non-const (si un jour tu veux modifier via itération)
    // ERM_Satellite* satBegin() { return _sats; }
    // ERM_Satellite* satEnd()   { return _sats + NB_SAT; }

private:
    ERM_Satellite _sats[NB_SAT];
};
