/*
 * ERM_CanService.cpp
 */

#include "ERM_CanService.h"
#include "Pins.h"
#include "ERM_Settings.h"
#include "ERM_CC_Manager.h"
#include "ERM_StopService.h"
#include "Variables.h"
#include "ProtocolCAN.h"
#include "Protocol.h"
#include "CanBus.h"
#include "Debug.h"

extern ERM_CC_Manager CC_Manager;
extern bool g_isTestMode;

/* ---------------------------------------------------------------------------
 * CONSTRUCTEUR
 * ------------------------------------------------------------------------- */
ERM_CanService::ERM_CanService()
{
    hasNewFrame = false;
    _lastRxTime = millis();
    _canOK = false;
}

/* ---------------------------------------------------------------------------
 * INITIALISATION
 * ------------------------------------------------------------------------- */
bool ERM_CanService::begin()
{
    LOG_INFO("ERM → CAN1 opérationnel");
    return true;
}

/* ---------------------------------------------------------------------------
 * RÉCEPTION CAN
 * ------------------------------------------------------------------------- */
void ERM_CanService::loop()
{
    CanMsg msg;

    if (CAN[1] && CAN[1]->receive(msg))
    {
        _lastRxTime = millis();
        _canOK = true;

        lastFrame = msg;
        hasNewFrame = true;

        LOG_CRITICAL_DCC("CAN RX → ID=0x%X len=%u", msg.id, msg.dlc);

        ERM_handleFrame(msg);
    }
}

/* ---------------------------------------------------------------------------
 * SUPERVISION BUS
 * ------------------------------------------------------------------------- */
bool ERM_CanService::checkBus(uint32_t timeoutMs)
{
    uint32_t age = millis() - _lastRxTime;
    _canOK = (age <= timeoutMs);

    if (!_canOK)
        LOG_WARN("ERM → aucune trame CAN depuis %u ms", age);

    return _canOK;
}

uint32_t ERM_CanService::lastRxAgeMs() const
{
    return millis() - _lastRxTime;
}

/* ---------------------------------------------------------------------------
 * ACCÈS À LA DERNIÈRE TRAME
 * ------------------------------------------------------------------------- */
bool ERM_CanService::getLastFrame(CanMsg &msg)
{
    if (hasNewFrame)
    {
        msg = lastFrame;
        hasNewFrame = false;
        return true;
    }
    return false;
}

/* ---------------------------------------------------------------------------
 * ENVOI PUBLIC
 * ------------------------------------------------------------------------- */
bool ERM_CanService::sendMessage(const CanMsg &msg)
{
    return ERM_sendFrame(msg);
}

/* ---------------------------------------------------------------------------
 * DÉCODAGE D’UNE TRAME CAN
 * ------------------------------------------------------------------------- */
void ERM_CanService::ERM_handleFrame(const CanMsg &msg)
{
    // 🔴 STOP global (11 bits)
    if (ProtocolCAN::isStop(msg.id))
    {
        ERM_StopService::onStopReceived();
        LOG_WARN("[CAN] STOP global reçu");
        return;
    }

    // 🟢 CLEAR STOP (11 bits)
    if (ProtocolCAN::isClearStop(msg.id))
    {
        ERM_StopService::onClearReceived();
        LOG_INFO("[CAN] CLEAR STOP reçu");
        return;
    }

    // 🔵 HEARTBEAT (11 bits)
    if (ProtocolCAN::isHeartbeat(msg.id))
    {
        uint16_t idExp = (uint16_t(msg.data[0]) << 8) | uint16_t(msg.data[1]);
        LOG_CRITICAL_DCC("[CAN] HEARTBEAT de %u", idExp);
        CC_Manager.updateHeartbeat(idExp);
        return;
    }

    // 🟧 Trame 29 bits → protocole Exploration
    auto f = ProtocolCAN::decode(msg.id);

    if (f.resp)
        return;

    LOG_CRITICAL_DCC("[CAN] ERM RX → cmd=0x%X src=%u prio=%u", f.cmd, f.src, f.prio);

    switch (static_cast<Cmd_CC_to_ERM>(f.cmd))
    {
    case Cmd_CC_to_ERM::TEST_BUS:
        ERM_handleCmdTestBus(f.src, f.prio);
        break;

    case Cmd_CC_to_ERM::REQUEST_ID:
        ERM_handleCmdRequestId(f.src, f.prio);
        sendTrackProfile(ERM_Settings::track_profile);
        break;

    default:
        LOG_CRITICAL_DCC("[CAN] Commande inconnue : 0x%X", f.cmd);
        break;
    }
}

/* ---------------------------------------------------------------------------
 * ENVOI BAS NIVEAU
 * ------------------------------------------------------------------------- */
bool ERM_CanService::ERM_sendFrame(const CanMsg &msg)
{
    if (g_isTestMode)
    {
        LOG_INFO("[TEST] Envoi CAN ignoré → ID=0x%X", msg.id);
        return true;
    }

    bool ok = CAN[1] ? CAN[1]->send(msg) : false;

    if (!ok)
        LOG_WARN("[CAN] Échec envoi ID=0x%X", msg.id);
    else
        LOG_CRITICAL_DCC("[CAN] TX → ID=0x%X len=%u", msg.id, msg.dlc);

    return ok;
}

/* ---------------------------------------------------------------------------
 * TEST BUS
 * ------------------------------------------------------------------------- */
void ERM_CanService::ERM_handleCmdTestBus(uint16_t idExp, uint8_t prio)
{
    LOG_INFO("[CAN] Test Bus reçu de %u", idExp);

    CanMsg msg = ProtocolCAN::makeMsg(
        prio,
        static_cast<uint16_t>(Cmd_ERM_to_CC::TEST_BUS_REPLY),
        true,
        idExp,
        {uint8_t(1)});

    ERM_sendFrame(msg);
    CC_Manager.addOrUpdate(idExp);
}

/* ---------------------------------------------------------------------------
 * REQUEST ID
 * ------------------------------------------------------------------------- */
void ERM_CanService::ERM_handleCmdRequestId(uint16_t idExp, uint8_t prio)
{
    LOG_INFO("[CAN] Demande d’ID reçue de %u", idExp);

    if (ERM_Settings::idCanton < 253)
    {
        CanMsg msg = ProtocolCAN::makeMsg(
            prio,
            static_cast<uint16_t>(Cmd_ERM_to_CC::REQUEST_ID_REPLY),
            true,
            idExp,
            {uint8_t(ERM_Settings::idCanton)});

        if (ERM_sendFrame(msg))
        {
            LOG_INFO("[CAN] Attribution ID=%u au Canton Controller %u",
                     ERM_Settings::idCanton, idExp);

            ERM_Settings::idCanton++;
            ERM_Settings::writeFile();

            // Envoyer le profil de voie
            sendTrackProfile(ERM_Settings::track_profile);
        }
    }
    else
    {
        LOG_WARN("[CAN] Plus d’ID disponibles !");
    }
}

/* ---------------------------------------------------------------------------
 * COMMANDES WEB → CAN
 * ------------------------------------------------------------------------- */
void ERM_CanService::sendWifiOnOff(bool on)
{
    LOG_INFO("[WEB→CAN] WIFI_ON_OFF = %s", on ? "true" : "false");

    CanMsg msg = ProtocolCAN::makeMsg(
        2,
        static_cast<uint16_t>(Cmd_ERM_to_CC::WIFI_ON_OFF),
        false,
        idMain,
        {uint8_t(on ? 1 : 0)});

    ERM_sendFrame(msg);
}

void ERM_CanService::sendExplorationOnOff(bool on)
{
    LOG_INFO("[WEB→CAN] EXPLORATION_ON_OFF = %s", on ? "true" : "false");

    CanMsg msg = ProtocolCAN::makeMsg(
        2,
        static_cast<uint16_t>(Cmd_ERM_to_CC::EXPLORATION_ON_OFF),
        false,
        idMain,
        {uint8_t(on ? 1 : 0)});

    ERM_sendFrame(msg);
}

void ERM_CanService::sendSaveAll()
{
    LOG_INFO("[WEB→CAN] SAVE_ALL");

    CanMsg msg = ProtocolCAN::makeMsg(
        2,
        static_cast<uint16_t>(Cmd_ERM_to_CC::SAVE_ALL),
        false,
        idMain,
        {});

    ERM_sendFrame(msg);
}

void ERM_CanService::sendRestartAll()
{
    LOG_INFO("[WEB→CAN] RESTART_ALL");

    CanMsg msg = ProtocolCAN::makeMsg(
        2,
        static_cast<uint16_t>(Cmd_ERM_to_CC::RESTART_ALL),
        false,
        idMain,
        {});

    ERM_sendFrame(msg);
}

void ERM_CanService::sendTrackProfile(uint8_t profile)
{
    LOG_INFO("[WEB→CAN] SET_PROFILE = %u", profile);

    CanMsg msg = ProtocolCAN::makeMsg(
        2,
        static_cast<uint16_t>(Cmd_ERM_to_CC::SET_PROFILE),
        false,
        idMain,
        {uint8_t(profile)});

    ERM_sendFrame(msg);
}
