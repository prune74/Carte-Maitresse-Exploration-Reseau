/*
DiscoveryMaster_SatManager.cpp / .h

🎯 Rôle
Gestionnaire centralisé des satellites Discovery.
Ce module maintient la liste des satellites connus, assure leur découverte,
leur mise à jour et fournit des fonctions d’accès rapides (recherche, comptage).
Il constitue la base de la supervision du réseau Discovery 2026.

📌 Fonctionnement
- Initialise un tableau fixe de NB_SAT satellites (DiscoveryMaster_Satellite).
- addOrUpdate() :
    • ajoute un nouveau satellite lorsqu’il apparaît sur le bus CAN
    • ignore les doublons
    • signale la découverte via Serial
- getById() :
    • retourne un pointeur vers un satellite existant
- count() :
    • retourne le nombre de satellites actuellement enregistrés

📌 Particularités
- La découverte des satellites est déclenchée par le driver CAN (DiscoveryMaster_CanService)
  lorsqu’une trame valide est reçue (ex : commande 0xB2).
- Le module est volontairement minimaliste pour l’instant :
    • begin() et loop() sont prévus pour accueillir plus tard :
        - heartbeat Discovery
        - timeout de communication
        - supervision avancée
        - gestion d’état (online/offline)
- Le tableau interne est statique pour garantir des performances constantes
  et éviter les allocations dynamiques.

🔗 Utilise
- DiscoveryMaster_Satellite (instances internes)
- DiscoveryMaster_CanService (déclenche la découverte)
- DiscoveryMaster_Settings (ID maître, configuration Discovery)
*/

#include "DiscoveryMaster_SatManager.h"

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

