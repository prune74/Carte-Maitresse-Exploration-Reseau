#include "ERS_Watchdog.h"
#include "Pins.h"
#include <Protocol.h>
#include "Variables.h"
#include "ERM_CC_Manager.h"
#include "ERM_CanService.h"
#include "ProtocolCAN.h"
#include "CanMsg.h"
#include "Debug.h"

extern ERM_CC_Manager CC_Manager;
extern ERM_CanService canService;

/* ---------------------------------------------------------------------------
 * INITIALISATION
 * ------------------------------------------------------------------------- */
void ERS_init()
{
    LOG_INFO("ERS → init interne OK");

    // Création du mutex si nécessaire
    if (ersHeartbeatMutex == nullptr)
    {
        ersHeartbeatMutex = xSemaphoreCreateMutexStatic(&ersHeartbeatMutexBuffer);
    }

    ers_enabled = true;
    ers_onlineCount = 0;
    ers_lastSupervisionTime = millis();

    // LED CC OFFLINE
    pinMode(PIN_LED_CC_OFFLINE, OUTPUT);
    digitalWrite(PIN_LED_CC_OFFLINE, LOW);
}

/* ---------------------------------------------------------------------------
 * MISE À JOUR HEARTBEAT
 * ------------------------------------------------------------------------- */
void ERS_registerHeartbeat(uint16_t ccId)
{
    if (!ers_enabled)
        return;

    CC_Manager.updateHeartbeat(ccId);

    // Mise à jour table interne ERS
    if (xSemaphoreTake(ersHeartbeatMutex, 0) == pdTRUE)
    {
        ers_lastHeartbeat[ccId] = millis();
        xSemaphoreGive(ersHeartbeatMutex);
    }

    ers_heartbeatCount++;
    ers_lastSatId = ccId;

    LOG_CRITICAL_DCC("ERS → heartbeat reçu de %u", ccId);
}

/* ---------------------------------------------------------------------------
 * STOP GLOBAL
 * ------------------------------------------------------------------------- */
void ERS_triggerEmergencyStop()
{
    // ID = Cmd_Global11::EMERGENCY_STOP (0x201)
    CanMsg msg(uint16_t(Cmd_Global11::EMERGENCY_STOP), {});

    canService.sendMessage(msg);
    LOG_WARN("[ERS] STOP global envoyé !");
}

/* ---------------------------------------------------------------------------
 * CC OFFLINE
 * ------------------------------------------------------------------------- */
void ERS_triggerCcOffline(uint16_t offlineId)
{
    // Payload = [ID_H][ID_L]
    uint8_t data[2] = {
        uint8_t(offlineId >> 8),
        uint8_t(offlineId & 0xFF)};

    // Commande = Cmd_ERM_to_CC::OFFLINE
    CanMsg msg = ProtocolCAN::makeMsg(
        2,
        static_cast<uint16_t>(Cmd_ERM_to_CC::OFFLINE),
        false,
        ERM_ID,
        {data[0], data[1]});

    canService.sendMessage(msg);
    LOG_WARN("[ERS] CC %u OFFLINE → notification envoyée !", offlineId);
}

/* ---------------------------------------------------------------------------
 * SUPERVISION
 * ------------------------------------------------------------------------- */
void ERS_supervise()
{
    if (!ers_enabled)
        return;

    ers_lastSupervisionTime = millis();

    CC_Manager.checkTimeouts(ERS_TIMEOUT_MS);

    uint16_t offlineId = 0;

    if (CC_Manager.hasOfflineCC(offlineId))
    {
        ers_timeoutCount++;

        LOG_ERROR("[ERS] CC %u OFFLINE → STOP global + notification CC", offlineId);

        ERS_triggerEmergencyStop();
        ERS_triggerCcOffline(offlineId);
        digitalWrite(PIN_LED_CC_OFFLINE, HIGH);
    }
    else
    {
        LOG_INFO("ERS → aucun Canton Controller offline");
        digitalWrite(PIN_LED_CC_OFFLINE, LOW);
    }
}
