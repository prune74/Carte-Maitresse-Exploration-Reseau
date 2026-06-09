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
// Initialise tous les slots satellites avec NO_ID.
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
// Pour l’instant, aucune supervision interne n’est nécessaire.
// La surveillance est effectuée par ERM_Surveillance.
void ERM_SatManager::loop()
{
    // Supervision externe (ERS)
}

// ---------------------------------------------------------------------------
// AJOUT / MISE À JOUR D’UN SATELLITE
// ---------------------------------------------------------------------------
// Enregistre un satellite s’il est nouveau, ou ignore s’il existe déjà.
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
// Met à jour l’état d’un satellite lorsqu’un heartbeat est reçu.
void ERM_SatManager::updateHeartbeat(uint16_t idSat)
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
// Déclare un satellite OFFLINE si aucun heartbeat n’a été reçu depuis timeoutMs.
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
// Retourne un pointeur vers un satellite, ou nullptr si absent.
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
// Compte les satellites ayant un ID valide.
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
// Retourne true si au moins un satellite est offline.
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
