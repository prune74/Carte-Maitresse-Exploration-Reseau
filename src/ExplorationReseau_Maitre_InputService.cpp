/*
 * ExplorationReseau_Maitre_InputService.cpp
 *
 * 🎯 Rôle
 * Gestion des entrées physiques de la Carte Maîtresse d’Exploration du Réseau.
 *
 * Ce module supervise notamment le bouton CLEAR STOP, permettant à l’opérateur
 * de lever manuellement un STOP global (0x202) sans passer par l’interface Web.
 */

#include "ExplorationReseau_Maitre_InputService.h"
#include "ExplorationReseau_Maitre_Pins.h"
#include "ExplorationReseau_Maitre_CanService.h"
#include "ExplorationReseau_Protocol.h"
#include "ExplorationReseau_Maitre_Config.h"
#include "Debug.h"

// Service CAN principal (version abrégée ERM)
extern ERM_CanService canService;

// ---------------------------------------------------------------------------
// Initialisation du service d’entrées
// ---------------------------------------------------------------------------
// Configure le bouton CLEAR STOP en entrée avec résistance pull-up interne.
void ERM_InputService::begin()
{
    pinMode(PIN_BTN_CLEAR_STOP, INPUT_PULLUP);

    LOG_INFO("ERM_InputService → bouton CLEAR STOP initialisé (pull-up)");
}

// ---------------------------------------------------------------------------
// Boucle de supervision du bouton CLEAR STOP
// ---------------------------------------------------------------------------
// Détection d’un front descendant (pression du bouton) avec anti-rebond simple.
void ERM_InputService::loop()
{
    bool cur = digitalRead(PIN_BTN_CLEAR_STOP);

    // Anti-rebond logiciel (20 ms)
    static uint32_t lastChange = 0;
    uint32_t now = millis();

    // Changement d’état + délai anti-rebond respecté
    if (cur != _prevBtn && (now - lastChange) > 20)
    {
        lastChange = now;

        // Front descendant → bouton pressé
        if (_prevBtn == true && cur == false)
        {
            // Construction d’une trame CLEAR STOP (11 bits)
            CanMsg msg(uint16_t(PROTOCOLCAN_ID_CLEAR_STOP), {});

            // Envoi via le service CAN
            canService.sendMessage(msg);

            LOG_INFO("[BTN] CLEAR STOP envoyé (bouton physique)");
        }

        _prevBtn = cur;
    }
}
