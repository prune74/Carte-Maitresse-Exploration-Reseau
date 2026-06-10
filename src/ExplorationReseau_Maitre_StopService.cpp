/*
 * ExplorationReseau_Maitre_StopService.cpp
 *
 * 🎯 Rôle
 * Implémentation du service STOP global pour la Carte Maîtresse ERM.
 *
 * Ce module garantit :
 *   • un état STOP cohérent et centralisé
 *   • une LED STOP toujours synchronisée
 *   • une gestion propre des trames CAN STOP (0x201) et CLEAR STOP (0x202)
 *   • une API simple pour WebHandler, InputService et CanService
 */

#include "ExplorationReseau_Maitre_StopService.h"
#include "ExplorationReseau_Maitre_Pins.h"
#include "ExplorationReseau_Maitre_CanService.h"
#include "ExplorationReseau_Protocol.h"
#include "Debug.h"

// Service CAN global
extern ERM_CanService canService;

// État interne STOP
bool ERM_StopService::_stopActive = false;

/* ---------------------------------------------------------------------------
 * 🟦 INITIALISATION
 * ------------------------------------------------------------------------- */
void ERM_StopService::begin()
{
    pinMode(PIN_LED_STOP, OUTPUT);
    digitalWrite(PIN_LED_STOP, LOW);
}

/* ---------------------------------------------------------------------------
 * 🟩 ÉTAT INTERNE
 * ------------------------------------------------------------------------- */
bool ERM_StopService::isStopActive()
{
    return _stopActive;
}

/* ---------------------------------------------------------------------------
 * 🟥 STOP GLOBAL (émission CAN + LED)
 * ------------------------------------------------------------------------- */
void ERM_StopService::triggerStop()
{
    if (_stopActive)
        return;

    _stopActive = true;
    digitalWrite(PIN_LED_STOP, HIGH);

    CanMsg msg(uint16_t(PROTOCOLCAN_ID_STOP), {});
    canService.sendMessage(msg);

    LOG_WARN("[STOP] STOP global déclenché");
}

/* ---------------------------------------------------------------------------
 * 🟦 CLEAR STOP (émission CAN + LED)
 * ------------------------------------------------------------------------- */
void ERM_StopService::clearStop()
{
    if (!_stopActive)
    {
        LOG_INFO("[STOP] CLEAR ignoré (STOP déjà levé)");
        return;
    }

    _stopActive = false;
    digitalWrite(PIN_LED_STOP, LOW);

    CanMsg msg(uint16_t(PROTOCOLCAN_ID_CLEAR_STOP), {});
    canService.sendMessage(msg);

    LOG_INFO("[STOP] CLEAR STOP envoyé");
}

/* ---------------------------------------------------------------------------
 * 🟥 STOP reçu via CAN
 * ------------------------------------------------------------------------- */
void ERM_StopService::onStopReceived()
{
    _stopActive = true;
    digitalWrite(PIN_LED_STOP, HIGH);

    LOG_WARN("[STOP] STOP global reçu via CAN");
}

/* ---------------------------------------------------------------------------
 * 🟦 CLEAR STOP reçu via CAN
 * ------------------------------------------------------------------------- */
void ERM_StopService::onClearReceived()
{
    _stopActive = false;
    digitalWrite(PIN_LED_STOP, LOW);

    LOG_INFO("[STOP] CLEAR STOP reçu via CAN");
}
