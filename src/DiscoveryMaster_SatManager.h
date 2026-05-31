#pragma once
#include <Arduino.h>
#include "DiscoveryMaster_Config.h"
#include "DiscoveryMaster_Satellite.h"

/*
DiscoveryMaster_SatManager.h — Version Discovery 2026

🎯 Rôle
Gestionnaire centralisé des satellites Discovery.
Il maintient la liste des satellites connus, met à jour leur état (online/offline),
et fournit une API simple pour la supervision (Watchdog Master).

📌 Fonctionnement
- Tableau statique de NB_SAT satellites (performant, déterministe).
- addOrUpdate() : ajoute un SA découvert sur le bus.
- updateHeartbeat() : met à jour lastSeen + online.
- checkTimeouts() : détecte les SA silencieux.
- hasOfflineSatellite() : utilisé par le Watchdog pour déclencher STOP global.

📌 Particularités
- Encapsulation stricte : _sats[] reste privé.
- API publique propre pour le Watchdog Discovery 2026.
*/

class DiscoveryMaster_SatManager
{
private:
    DiscoveryMaster_Satellite _sats[NB_SAT];

public:
    DiscoveryMaster_SatManager();

    void begin();
    void loop();

    void addOrUpdate(uint16_t idSat);
    void updateHeartbeat(uint16_t idSat);
    void checkTimeouts(uint32_t timeoutMs);

    DiscoveryMaster_Satellite *getById(uint16_t idSat);
    uint8_t count() const;

    // Accès en lecture seule (optionnel)
    const DiscoveryMaster_Satellite* getAll() const { return _sats; }

    // ⭐ API pour Watchdog Discovery 2026
    // Retourne true si un SA est offline et renvoie son ID
    bool hasOfflineSatellite(uint16_t &offlineId) const;
};
