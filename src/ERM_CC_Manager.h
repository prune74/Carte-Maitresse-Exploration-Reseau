#pragma once
#include <Arduino.h>
#include "ERM_Config.h"
#include "ERM_CantonController.h"

/*
 * 🎯 Rôle
 * Gestionnaire des Canton Controller du réseau d’exploration.
 *
 * Ce module maintient :
 *   • un tableau fixe de ERM_CantonController
 *   • la découverte automatique
 *   • les heartbeats
 *   • l’état online/offline
 *   • l’interface de diagnostic pour ERS
 *
 * API modernisée :
 *   • begin() / end() → itération style STL
 */

class ERM_CC_Manager
{
public:
    ERM_CC_Manager();

    void begin();
    void loop();

    void addOrUpdate(uint16_t idCC);
    void updateHeartbeat(uint16_t idCC);
    void checkTimeouts(uint32_t timeoutMs);

    ERM_CantonController *getById(uint16_t idCC);
    uint8_t count() const;

    bool hasOfflineCC(uint16_t &offlineId) const;

    // -----------------------------------------------------------------------
    // 🆕 API moderne : itération style STL
    // -----------------------------------------------------------------------
    const ERM_CantonController *ccBegin() const { return _cantonController; }
    const ERM_CantonController *ccEnd() const { return _cantonController + NB_CC; }

    // Version non-const (si un jour tu veux modifier via itération)
    // ERM_CantonController* ccBegin() { return _cantonController; }
    // ERM_CantonController* ccEnd()   { return _cantonController + NB_CC; }

private:
    ERM_CantonController _cantonController[NB_CC];
};
