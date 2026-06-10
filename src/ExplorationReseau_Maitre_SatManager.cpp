#include "ExplorationReseau_Maitre_SatManager.h"
#include "Debug.h"

/*
 * ExplorationReseau_Maitre_SatManager.cpp
 *
 * 🎯 Rôle
 * Gestionnaire central des satellites du réseau d’exploration (ERM).
 *
 * Ce module assure :
 *   • la découverte automatique des satellites
 *   • la mise à jour des heartbeats
 *   • la supervision online/offline
 *   • l’interface de diagnostic pour la surveillance (ERS)
 *
 * Il utilise un tableau statique de ERM_Satellite pour garantir
 * une exécution déterministe et sans allocation dynamique.
 */

// ---------------------------------------------------------------------------
// CONSTRUCTEUR
// ---------------------------------------------------------------------------
ERM_SatManager::ERM_SatManager()
{
    for (uint8_t i = 0; i < NB_SAT; i++)
        _sats[i].id = NO_ID;
}

// ---------------------------------------------------------------------------
// INITIALISATION
// ---------------------------------------------------------------------------
void ERM_SatManager::begin()
{
    LOG_INFO("ERM_SatManager → initialisé (%u slots)", NB_SAT);
}

// ---------------------------------------------------------------------------
// BOUCLE PRINCIPALE
// ---------------------------------------------------------------------------
// Supervision déléguée à ERS. Rien à faire ici.
void ERM_SatManager::loop()
{
    // Supervision externe (ERS)
}

// ---------------------------------------------------------------------------
// AJOUT / MISE À JOUR D’UN SATELLITE
// ---------------------------------------------------------------------------
void ERM_SatManager::addOrUpdate(uint16_t idSat)
{
    if (idSat == NO_ID)
        return;

    // Déjà connu ?
    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id == idSat)
        {
            // Satellite déjà enregistré
            return;
        }
    }

    // Nouveau satellite → premier slot libre
    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id == NO_ID)
        {
            _sats[i].id       = idSat;
            _sats[i].online   = true;
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
void ERM_SatManager::updateHeartbeat(uint16_t idSat)
{
    for (uint8_t i = 0; i < NB_SAT; i++)
    {
        if (_sats[i].id == idSat)
        {
            bool wasOffline = !_sats[i].online;

            _sats[i].lastSeen = millis();
            _sats[i].online   = true;

            if (wasOffline)
                LOG_INFO("SAT %u → ONLINE (heartbeat)", idSat);
            else
                LOG_CRITICAL_DCC("SAT %u → heartbeat", idSat);

            return;
        }
    }

    // Satellite inconnu → découverte automatique
    LOG_CRITICAL_DCC("SAT %u → heartbeat inconnu → ajout", idSat);
    addOrUpdate(idSat);
}

// ---------------------------------------------------------------------------
// SUPERVISION DES TIMEOUTS
// ---------------------------------------------------------------------------
void ERM_SatManager::checkTimeouts(uint32_t timeoutMs)
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
ERM_Satellite *ERM_SatManager::getById(uint16_t idSat)
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
uint8_t ERM_SatManager::count() const
{
    uint8_t c = 0;
    for (uint8_t i = 0; i < NB_SAT; i++)
        if (_sats[i].id != NO_ID)
            c++;
    return c;
}

// ---------------------------------------------------------------------------
// WATCHDOG : DÉTECTION D’UN SATELLITE OFFLINE
// ---------------------------------------------------------------------------
bool ERM_SatManager::hasOfflineSatellite(uint16_t &offlineId) const
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
