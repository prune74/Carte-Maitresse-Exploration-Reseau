#include "DiscoveryMaster_SatManager.h"
#include "Debug.h"

/*
DiscoveryMaster_SatManager.cpp — Version Discovery 2026

🎯 Rôle
Gestionnaire central des satellites Discovery :
- découverte
- heartbeat
- supervision online/offline
- interface Watchdog
*/

DiscoveryMaster_SatManager::DiscoveryMaster_SatManager()
{
    for (uint8_t i = 0; i < NB_SAT; i++)
        _sats[i].id = NO_ID;
}

void DiscoveryMaster_SatManager::begin()
{
    LOG_INFO("SatManager → initialisé (%u slots)", NB_SAT);
}

void DiscoveryMaster_SatManager::loop()
{
    // Rien ici pour l’instant (supervision externe)
}

// ---------------------------------------------------------------------------
// AJOUT / MISE À JOUR D’UN SATELLITE
// ---------------------------------------------------------------------------
void DiscoveryMaster_SatManager::addOrUpdate(uint16_t idSat)
{
    if (idSat == NO_ID)
        return;

    // Déjà connu ?
    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id == idSat)
        {
            // Rien à faire : déjà enregistré
            return;
        }
    }

    // Nouveau satellite → premier slot libre
    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id == NO_ID)
        {
            _sats[i].id = idSat;
            _sats[i].online = true;
            _sats[i].lastSeen = millis();

            LOG_INFO("SAT %u → découvert", idSat);
            return;
        }
    }

    LOG_WARN("SAT → liste pleine, impossible d’ajouter %u", idSat);
}

// ---------------------------------------------------------------------------
// HEARTBEAT
// ---------------------------------------------------------------------------
void DiscoveryMaster_SatManager::updateHeartbeat(uint16_t idSat)
{
    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id == idSat)
        {
            bool wasOffline = !_sats[i].online;

            _sats[i].lastSeen = millis();
            _sats[i].online = true;

            if (wasOffline)
                LOG_INFO("SAT %u → ONLINE (heartbeat)", idSat);
            else
                LOG_VERBOSE("SAT %u → heartbeat", idSat);

            return;
        }
    }

    // Satellite inconnu → découverte automatique
    LOG_VERBOSE("SAT %u → heartbeat inconnu → ajout", idSat);
    addOrUpdate(idSat);
}

// ---------------------------------------------------------------------------
// SUPERVISION DES TIMEOUTS
// ---------------------------------------------------------------------------
void DiscoveryMaster_SatManager::checkTimeouts(uint32_t timeoutMs)
{
    uint32_t now = millis();

    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id != NO_ID && _sats[i].online)
        {
            uint32_t age = now - _sats[i].lastSeen;

            if (age > timeoutMs)
            {
                _sats[i].online = false;
                LOG_WARN("SAT %u → OFFLINE (timeout %u ms)", _sats[i].id, age);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// ACCÈS PAR ID
// ---------------------------------------------------------------------------
DiscoveryMaster_Satellite *DiscoveryMaster_SatManager::getById(uint16_t idSat)
{
    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id == idSat)
            return &_sats[i];
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// NOMBRE DE SATELLITES ENREGISTRÉS
// ---------------------------------------------------------------------------
uint8_t DiscoveryMaster_SatManager::count() const
{
    uint8_t c = 0;
    for (uint8_t i = 0; i < NB_SAT; i++)
        if (_sats[i].id != NO_ID)
            c++;
    return c;
}

// ---------------------------------------------------------------------------
// WATCHDOG : détecter un satellite offline
// ---------------------------------------------------------------------------
bool DiscoveryMaster_SatManager::hasOfflineSatellite(uint16_t &offlineId) const
{
    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id != NO_ID && !_sats[i].online)
        {
            offlineId = _sats[i].id;
            return true;
        }
    }
    return false;
}
