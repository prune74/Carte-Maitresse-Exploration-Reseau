/*
 * DiscoveryMaster_InputService.cpp — Version Discovery 2026
 *
 * 🎯 Rôle
 * Gestion des entrées physiques du Master Discovery.
 * Ce module gère notamment le bouton CLEAR STOP, permettant à l’opérateur
 * de lever manuellement un STOP global (0x202) sans passer par l’interface Web.
 *
 * 📌 Fonctionnement
 * - Lecture périodique du bouton CLEAR STOP (entrée pull-up)
 * - Détection d’un front descendant (pression)
 * - Anti-rebond logiciel (20 ms)
 * - Envoi d’une trame CAN CLEAR STOP (0x202) via canService
 *
 * 📌 Particularités
 * - Module totalement indépendant du main, du WebHandler et du SatManager
 * - Architecture modulaire Discovery 2026 : chaque service a son rôle
 * - Le bouton fonctionne même si le WiFi ou l’UI Web sont indisponibles
 */

#include "DiscoveryMaster_InputService.h"
#include "DiscoveryMaster_Pins.h"
#include "DiscoveryMaster_CanService.h"
#include "Discovery_Protocol.h"

extern DiscoveryMaster_CanService canService;

// ---------------------------------------------------------------------------
// Initialisation du service d’entrées
// ---------------------------------------------------------------------------
void DiscoveryMaster_InputService::begin()
{
    // Bouton CLEAR STOP en pull-up interne
    // HIGH = relâché, LOW = pressé
    pinMode(PIN_BTN_CLEAR_STOP, INPUT_PULLUP);
}

// ---------------------------------------------------------------------------
// Boucle de supervision du bouton CLEAR STOP
// ---------------------------------------------------------------------------
void DiscoveryMaster_InputService::loop()
{
    bool cur = digitalRead(PIN_BTN_CLEAR_STOP);

    // Anti-rebond simple (20 ms)
    static uint32_t lastChange = 0;
    uint32_t now = millis();

    // Changement d’état détecté + délai anti-rebond respecté
    if (cur != _prevBtn && (now - lastChange) > 20)
    {
        lastChange = now;

        // Front descendant → bouton pressé
        if (_prevBtn == true && cur == false)
        {
            // Construction de la trame CLEAR STOP (0x202)
            CANMessage msg;
            msg.id  = DISCOVERY_CAN_ID_CLEAR_STOP; // 0x202
            msg.ext = false;                       // 11 bits
            msg.len = 0;                           // aucune data

            // Envoi sur le bus CAN Discovery
            canService.sendMessage(msg);

            Serial.println("[MASTER] CLEAR STOP envoyé (bouton)");
        }

        // Mise à jour de l’état précédent
        _prevBtn = cur;
    }
}
