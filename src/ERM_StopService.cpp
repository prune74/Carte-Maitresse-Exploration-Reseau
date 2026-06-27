/*
 * ERM_StopService.cpp
 */

#include "ERM_StopService.h"
#include "Pins.h"
#include "ERM_CanService.h"
#include "Protocol.h"
#include "Debug.h"

extern ERM_CanService canService;

// État interne STOP
bool ERM_StopService::_stopActive = false;

/* ---------------------------------------------------------------------------
 * INITIALISATION
 * ------------------------------------------------------------------------- */
void ERM_StopService::begin()
{
    pinMode(PIN_LED_STOP, OUTPUT);
    digitalWrite(PIN_LED_STOP, LOW);
}

/* ---------------------------------------------------------------------------
 * ÉTAT INTERNE
 * ------------------------------------------------------------------------- */
bool ERM_StopService::isStopActive()
{
    return _stopActive;
}

/* ---------------------------------------------------------------------------
 * STOP GLOBAL (émission CAN + LED)
 * ------------------------------------------------------------------------- */
void ERM_StopService::triggerStop()
{
    if (_stopActive)
        return;

    _stopActive = true;
    digitalWrite(PIN_LED_STOP, HIGH);

    CanMsg msg(uint16_t(Cmd_Global11::EMERGENCY_STOP), {});
    canService.sendMessage(msg);

    LOG_WARN("[STOP] STOP global déclenché");
}

/* ---------------------------------------------------------------------------
 * CLEAR STOP (émission CAN + LED)
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

    // Auto-reset LED_CC_OFFLINE
    digitalWrite(PIN_LED_CC_OFFLINE, LOW);

    CanMsg msg(uint16_t(Cmd_Global11::CLEAR_STOP), {});
    canService.sendMessage(msg);

    LOG_INFO("[STOP] CLEAR STOP envoyé");
}

/* ---------------------------------------------------------------------------
 * STOP reçu via CAN
 * ------------------------------------------------------------------------- */
void ERM_StopService::onStopReceived()
{
    _stopActive = true;
    digitalWrite(PIN_LED_STOP, HIGH);

    LOG_WARN("[STOP] STOP global reçu via CAN");
}

/* ---------------------------------------------------------------------------
 * CLEAR STOP reçu via CAN
 * ------------------------------------------------------------------------- */
void ERM_StopService::onClearReceived()
{
    _stopActive = false;
    digitalWrite(PIN_LED_STOP, LOW);

    // Auto-reset LED_CC_OFFLINE
    digitalWrite(PIN_LED_CC_OFFLINE, LOW);

    LOG_INFO("[STOP] CLEAR STOP reçu via CAN");
}
