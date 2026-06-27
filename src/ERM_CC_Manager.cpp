#include "ERM_CC_Manager.h"
#include "Debug.h"

/*
 * ExplorationReseau_Maitre_SatManager.cpp
 *
 * 🎯 Rôle
 * Gestionnaire central des Canton Controller du réseau d’exploration (ERM).
 *
 * Ce module assure :
 *   • la découverte automatique des Canton Controller
 *   • la mise à jour des heartbeats
 *   • la supervision online/offline
 *   • l’interface de diagnostic pour la surveillance (ERS)
 *
 * Il utilise un tableau statique de ERM_CantonController pour garantir
 * une exécution déterministe et sans allocation dynamique.
 */

// ---------------------------------------------------------------------------
// CONSTRUCTEUR
// ---------------------------------------------------------------------------
ERM_CC_Manager::ERM_CC_Manager()
{
    for (uint8_t i = 0; i < NB_CC; i++)
        _cantonController[i].id = NO_ID;
}

// ---------------------------------------------------------------------------
// INITIALISATION
// ---------------------------------------------------------------------------
void ERM_CC_Manager::begin()
{
    LOG_INFO("ERM_CC_Manager → initialisé (%u slots)", NB_CC);
}

// ---------------------------------------------------------------------------
// BOUCLE PRINCIPALE
// ---------------------------------------------------------------------------
// Supervision déléguée à ERS. Rien à faire ici.
void ERM_CC_Manager::loop()
{
    // Supervision externe (ERS)
}

// ---------------------------------------------------------------------------
// AJOUT / MISE À JOUR D’UN CANTON CONTROLLER
// ---------------------------------------------------------------------------
void ERM_CC_Manager::addOrUpdate(uint16_t idCC)
{
    if (idCC == NO_ID)
        return;

    // Déjà connu ?
    for (uint8_t i = 0; i < NB_CC; i++)
    {
        if (_cantonController[i].id == idCC)
        {
            // Canton Controller déjà enregistré
            return;
        }
    }

    // Nouveau Canton Controller → premier slot libre
    for (uint8_t i = 0; i < NB_CC; i++)
    {
        if (_cantonController[i].id == NO_ID)
        {
            _cantonController[i].id = idCC;
            _cantonController[i].online = true;
            _cantonController[i].lastSeen = millis();

            LOG_INFO("CC %u → découvert", idCC);
            return;
        }
    }

    LOG_WARN("CC → liste pleine, impossible d’ajouter %u", idCC);
}

// ---------------------------------------------------------------------------
// HEARTBEAT
// ---------------------------------------------------------------------------
void ERM_CC_Manager::updateHeartbeat(uint16_t idCC)
{
    for (uint8_t i = 0; i < NB_CC; i++)
    {
        if (_cantonController[i].id == idCC)
        {
            bool wasOffline = !_cantonController[i].online;

            _cantonController[i].lastSeen = millis();
            _cantonController[i].online = true;

            if (wasOffline)
                LOG_INFO("CC %u → ONLINE (heartbeat)", idCC);
            else
                LOG_CRITICAL_DCC("CC %u → heartbeat", idCC);

            return;
        }
    }

    // Canton Controller inconnu → découverte automatique
    LOG_CRITICAL_DCC("CC %u → heartbeat inconnu → ajout", idCC);
    addOrUpdate(idCC);
}

// ---------------------------------------------------------------------------
// SUPERVISION DES TIMEOUTS
// ---------------------------------------------------------------------------
void ERM_CC_Manager::checkTimeouts(uint32_t timeoutMs)
{
    uint32_t now = millis();

    for (uint8_t i = 0; i < NB_CC; i++)
    {
        if (_cantonController[i].id != NO_ID && _cantonController[i].online)
        {
            uint32_t age = now - _cantonController[i].lastSeen;

            if (age > timeoutMs)
            {
                _cantonController[i].online = false;
                LOG_WARN("CC %u → OFFLINE (timeout %u ms)", _cantonController[i].id, age);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// ACCÈS PAR ID
// ---------------------------------------------------------------------------
ERM_CantonController *ERM_CC_Manager::getById(uint16_t idCC)
{
    for (uint8_t i = 0; i < NB_CC; i++)
    {
        if (_cantonController[i].id == idCC)
            return &_cantonController[i];
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// NOMBRE DE SATELLITES ENREGISTRÉS
// ---------------------------------------------------------------------------
uint8_t ERM_CC_Manager::count() const
{
    uint8_t c = 0;
    for (uint8_t i = 0; i < NB_CC; i++)
        if (_cantonController[i].id != NO_ID)
            c++;
    return c;
}

// ---------------------------------------------------------------------------
// WATCHDOG : DÉTECTION D’UN CANTON CONTROLLER OFFLINE
// ---------------------------------------------------------------------------
bool ERM_CC_Manager::hasOfflineCC(uint16_t &offlineId) const
{
    for (uint8_t i = 0; i < NB_CC; i++)
    {
        if (_cantonController[i].id != NO_ID && !_cantonController[i].online)
        {
            offlineId = _cantonController[i].id;
            return true;
        }
    }
    return false;
}
