/*
 * ExplorationReseau_Maitre_InputService.cpp
 *
 * 🎯 Rôle
 * Gestion des entrées physiques de la Carte Maîtresse d’Exploration du Réseau.
 *
 * Ce module supervise le bouton CLEAR STOP :
 *   • Appui court → demande de levée du STOP global (ID 0x202)
 *   • Anti-rebond logiciel léger
 *   • Lecture non bloquante
 *
 * La logique STOP/CLEAR STOP est centralisée dans ERM_StopService.
 * Le bouton ne peut lever STOP que si STOP est réellement actif.
 */

#include "ExplorationReseau_Maitre_InputService.h"
#include "ExplorationReseau_Maitre_Pins.h"
#include "ExplorationReseau_Maitre_StopService.h"
#include "Debug.h"

/* ---------------------------------------------------------------------------
 * 🟦 INITIALISATION DU SERVICE D’ENTRÉES
 * ------------------------------------------------------------------------- */
void ERM_InputService::begin()
{
    pinMode(PIN_BTN_CLEAR_STOP, INPUT_PULLUP);

    LOG_INFO("ERM_InputService → bouton CLEAR STOP initialisé (pull-up)");
}

/* ---------------------------------------------------------------------------
 * 🟩 BOUCLE DE SUPERVISION DU BOUTON CLEAR STOP
 *
 * Détection d’un front descendant (pression du bouton) avec anti-rebond.
 * Aucun traitement lourd, aucune attente bloquante.
 *
 * CLEAR STOP n’est envoyé que si STOP est actif.
 * L’envoi CAN est délégué à ERM_StopService.
 * ------------------------------------------------------------------------- */
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
            // Vérifie si STOP est actif avant CLEAR STOP
            if (ERM_StopService::isStopActive())
            {
                ERM_StopService::clearStop();
                LOG_INFO("[BTN] CLEAR STOP demandé (STOP actif)");
            }
            else
            {
                LOG_INFO("[BTN] CLEAR STOP ignoré (STOP non actif)");
            }
        }

        _prevBtn = cur;
    }
}
