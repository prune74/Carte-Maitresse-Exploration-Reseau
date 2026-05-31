#include "DiscoveryMaster_SatManager.h"

/*
DiscoveryMaster_SatManager.cpp — Version Discovery 2026

🎯 Rôle
Implémentation du gestionnaire de satellites Discovery.
Ce module est la source de vérité du réseau Discovery 2026 :
- découverte des SA
- mise à jour des heartbeat
- gestion online/offline
- supervision via Watchdog Master
*/

DiscoveryMaster_SatManager::DiscoveryMaster_SatManager()
{
    for (uint8_t i = 0; i < NB_SAT; i++)
        _sats[i].id = NO_ID;
}

void DiscoveryMaster_SatManager::begin()
{
    // Rien pour l’instant
}

void DiscoveryMaster_SatManager::loop()
{
    // Optionnel si tu veux une supervision interne
}

void DiscoveryMaster_SatManager::addOrUpdate(uint16_t idSat)
{
    if (idSat == NO_ID)
        return;

    // Déjà connu ?
    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id == idSat)
            return;
    }

    // Sinon on ajoute dans le premier slot libre
    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id == NO_ID)
        {
            _sats[i].id = idSat;
            _sats[i].online = true;
            _sats[i].lastSeen = millis();

            Serial.printf("Nouveau satellite découvert : %d\n", idSat);
            return;
        }
    }

    Serial.println("⚠ Liste des satellites pleine !");
}

void DiscoveryMaster_SatManager::updateHeartbeat(uint16_t idSat)
{
    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id == idSat)
        {
            _sats[i].lastSeen = millis();
            _sats[i].online = true;
            return;
        }
    }

    // Satellite inconnu → on l’ajoute
    addOrUpdate(idSat);
}

void DiscoveryMaster_SatManager::checkTimeouts(uint32_t timeoutMs)
{
    uint32_t now = millis();

    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id != NO_ID && _sats[i].online)
        {
            if (now - _sats[i].lastSeen > timeoutMs)
                _sats[i].online = false;
        }
    }
}

DiscoveryMaster_Satellite *DiscoveryMaster_SatManager::getById(uint16_t idSat)
{
    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id == idSat)
            return &_sats[i];
    }
    return nullptr;
}

uint8_t DiscoveryMaster_SatManager::count() const
{
    uint8_t c = 0;
    for (uint8_t i = 0; i < NB_SAT; i++)
        if (_sats[i].id != NO_ID)
            c++;
    return c;
}

/*
⭐ Méthode ajoutée pour Watchdog Discovery 2026
Permet au Watchdog Master de détecter un SA offline
sans accéder directement à _sats[] (encapsulation respectée).
*/
bool DiscoveryMaster_SatManager::hasOfflineSatellite(uint16_t &offlineId) const
{
    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id != NO_ID && _sats[i].online == false)
        {
            offlineId = _sats[i].id;
            return true;
        }
    }
    return false;
}
