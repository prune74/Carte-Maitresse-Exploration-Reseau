#include "DiscoveryMaster_CanService.h"
#include "DiscoveryMaster_Pins.h"
#include "DiscoveryMaster_Settings.h"
#include "DiscoveryMaster_SatManager.h"
#include "ProtocolCAN.h"
#include "Discovery_Protocol.h"
#include "CanBus.h"

extern uint16_t idMain;
extern DiscoveryMaster_SatManager satManager;

// CAN1 = MCP2515 externe
extern CanBus CAN[]; // défini dans CanInit.cpp / CanUniversal

// ---------------------------------------------------------------------------
// CONSTRUCTEUR
// ---------------------------------------------------------------------------
DiscoveryMaster_CanService::DiscoveryMaster_CanService()
{
    hasNewFrame = false;

    // Supervision CAN
    _lastRxTime = millis();
    _canOK = false;
}

// ---------------------------------------------------------------------------
// INITIALISATION (CanUniversal déjà fait ailleurs)
// ---------------------------------------------------------------------------
bool DiscoveryMaster_CanService::begin()
{
    Serial.println("[MASTER][CAN] Service DiscoveryMaster_CanService prêt (CanUniversal)");
    return true;
}

// ---------------------------------------------------------------------------
// LOOP CAN
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::loop()
{
    CanMsg msg;

    // CAN[1] = MCP2515
    if (CAN[1].receive(msg))
    {
        _lastRxTime = millis();
        _canOK = true;

        lastFrame = msg;
        hasNewFrame = true;

        handleFrame(msg);
    }
}

// ---------------------------------------------------------------------------
// SUPERVISION
// ---------------------------------------------------------------------------
bool DiscoveryMaster_CanService::checkBus(uint32_t timeoutMs)
{
    uint32_t age = millis() - _lastRxTime;
    _canOK = (age <= timeoutMs);
    return _canOK;
}

uint32_t DiscoveryMaster_CanService::lastRxAgeMs() const
{
    return millis() - _lastRxTime;
}

// ---------------------------------------------------------------------------
// WATCHDOG
// ---------------------------------------------------------------------------
bool DiscoveryMaster_CanService::getLastFrame(CanMsg &msg)
{
    if (hasNewFrame)
    {
        msg = lastFrame;
        hasNewFrame = false;
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// ENVOI PUBLIC
// ---------------------------------------------------------------------------
bool DiscoveryMaster_CanService::sendMessage(const CanMsg &msg)
{
    return sendFrame(msg);
}

// ---------------------------------------------------------------------------
// DÉCODAGE TRAME
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::handleFrame(const CanMsg &msg)
{

    // 🟥 11 bits : STOP
    if (ProtocolCAN::isStop(msg.id))
    {
        digitalWrite(PIN_LED_STOP, HIGH);
        Serial.println("[MASTER][CAN] STOP global reçu");
        return;
    }

    // 🟩 11 bits : CLEAR STOP
    if (ProtocolCAN::isClearStop(msg.id))
    {
        digitalWrite(PIN_LED_STOP, LOW);
        Serial.println("[MASTER][CAN] CLEAR STOP reçu");
        return;
    }

    // 🟦 11 bits : HEARTBEAT
    if (ProtocolCAN::isHeartbeat(msg.id))
    {
        uint16_t idExp = (uint16_t(msg.data[0]) << 8) | uint16_t(msg.data[1]);
        satManager.updateHeartbeat(idExp);
        return;
    }

    // 🟧 29 bits : protocole Discovery
    auto f = ProtocolCAN::decode(msg.id);

    if (f.resp)
        return;

    switch (f.cmd)
    {
    case CMD_SAT_TEST_BUS:
        handleCmdTestBus(f.src, f.prio);
        break;

    case CMD_SAT_REQUEST_ID:
        handleCmdRequestId(f.src, f.prio);
        break;

    case CMD_SAT_HEARTBEAT:
        satManager.updateHeartbeat(f.src);
        break;

    default:
        break;
    }
}

// ---------------------------------------------------------------------------
// ENVOI BAS NIVEAU
// ---------------------------------------------------------------------------
bool DiscoveryMaster_CanService::sendFrame(const CanMsg &msg)
{
    return CAN[1].send(msg);
}

// ---------------------------------------------------------------------------
// CMD_SAT_TEST_BUS
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::handleCmdTestBus(uint16_t idExp, uint8_t priorite)
{
    CanMsg msg = ProtocolCAN::makeMsg(
        priorite,
        CMD_SAT_TEST_BUS_REPLY,
        true,
        idExp,
        {uint8_t(1)});

    sendFrame(msg);
    satManager.addOrUpdate(idExp);
}

// ---------------------------------------------------------------------------
// CMD_SAT_REQUEST_ID
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::handleCmdRequestId(uint16_t idExp, uint8_t priorite)
{
    if (DiscoveryMaster_Settings::idNode < 253)
    {

        CanMsg msg = ProtocolCAN::makeMsg(
            priorite,
            CMD_SAT_REQUEST_ID_REPLY,
            true,
            idExp,
            {uint8_t(DiscoveryMaster_Settings::idNode)});

        if (sendFrame(msg))
        {
            DiscoveryMaster_Settings::idNode++;
            DiscoveryMaster_Settings::writeFile();
        }
    }
}

// ---------------------------------------------------------------------------
// COMMANDES WEB
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::sendWifiOnOff(bool on)
{
    CanMsg msg = ProtocolCAN::makeMsg(
        2, CMD_WIFI_ON_OFF, false, idMain, {uint8_t(on ? 1 : 0)});
    sendFrame(msg);
}

void DiscoveryMaster_CanService::sendDiscoveryOnOff(bool on)
{
    CanMsg msg = ProtocolCAN::makeMsg(
        2, CMD_DISCOVERY_ON_OFF, false, idMain, {uint8_t(on ? 1 : 0)});
    sendFrame(msg);
}

void DiscoveryMaster_CanService::sendSaveAll()
{
    CanMsg msg = ProtocolCAN::makeMsg(
        2, CMD_SAVE_ALL, false, idMain, {});
    sendFrame(msg);
}

void DiscoveryMaster_CanService::sendRestartAll()
{
    CanMsg msg = ProtocolCAN::makeMsg(
        2, CMD_RESTART_ALL, false, idMain, {});
    sendFrame(msg);
}

void DiscoveryMaster_CanService::sendTrackProfile(uint8_t profile)
{
    CanMsg msg = ProtocolCAN::makeMsg(
        2, CMD_SET_PROFILE, false, idMain, {uint8_t(profile)});
    sendFrame(msg);
}
