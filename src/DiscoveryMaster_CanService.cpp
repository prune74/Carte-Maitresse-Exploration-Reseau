#include "DiscoveryMaster_CanService.h"
#include "DiscoveryMaster_Pins.h"
#include "DiscoveryMaster_Settings.h"
#include "DiscoveryMaster_SatManager.h"
#include "ProtocolCAN.h"
#include "Discovery_Protocol.h"
#include "CanBus.h"
#include "Debug.h"

extern uint16_t idMain;                         // ID du maître Discovery (254 par défaut)
extern DiscoveryMaster_SatManager satManager;   // Gestionnaire de satellites
extern bool g_isTestMode;                       // Indication du mode test (FakeDCC) pour activer le loopback

extern CanBus CAN[];                            // CAN1 = MCP2515 externe

// ---------------------------------------------------------------------------
// CONSTRUCTEUR
// ---------------------------------------------------------------------------
DiscoveryMaster_CanService::DiscoveryMaster_CanService()
{
    hasNewFrame = false;
    _lastRxTime = millis();
    _canOK = false;
}

// ---------------------------------------------------------------------------
// INITIALISATION
// ---------------------------------------------------------------------------
bool DiscoveryMaster_CanService::begin()
{
    LOG_INFO("MasterConfig → CAN1 prêt (via CanInit)");
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

        LOG_VERBOSE("CAN RX → ID=0x%X len=%u", msg.id, msg.dlc);

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

    if (!_canOK)
        LOG_WARN("CAN1 supervision → aucune trame depuis %u ms", age);

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
    // 🟥 STOP global
    if (ProtocolCAN::isStop(msg.id))
    {
        digitalWrite(PIN_LED_STOP, HIGH);
        LOG_WARN("[CAN] STOP global reçu");
        return;
    }

    // 🟩 CLEAR STOP
    if (ProtocolCAN::isClearStop(msg.id))
    {
        digitalWrite(PIN_LED_STOP, LOW);
        LOG_INFO("[CAN] CLEAR STOP reçu");
        return;
    }

    // 🟦 HEARTBEAT (11 bits)
    if (ProtocolCAN::isHeartbeat(msg.id))
    {
        uint16_t idExp = (uint16_t(msg.data[0]) << 8) | uint16_t(msg.data[1]);
        LOG_VERBOSE("[CAN] HEARTBEAT reçu de %u", idExp);
        satManager.updateHeartbeat(idExp);
        return;
    }

    // 🟧 29 bits : protocole Discovery
    auto f = ProtocolCAN::decode(msg.id);

    if (f.resp)
        return;

    LOG_VERBOSE("[CAN] Discovery RX → cmd=0x%X src=%u prio=%u", f.cmd, f.src, f.prio);

    switch (f.cmd)
    {
    case CMD_SAT_TEST_BUS:
        handleCmdTestBus(f.src, f.prio);
        break;

    case CMD_SAT_REQUEST_ID:
        handleCmdRequestId(f.src, f.prio);
        break;

    case CMD_SAT_HEARTBEAT:
        LOG_VERBOSE("[CAN] HEARTBEAT Discovery de %u", f.src);
        satManager.updateHeartbeat(f.src);
        break;

    default:
        LOG_VERBOSE("[CAN] Commande Discovery ignorée : 0x%X", f.cmd);
        break;
    }
}

// ---------------------------------------------------------------------------
// ENVOI BAS NIVEAU
// ---------------------------------------------------------------------------
bool DiscoveryMaster_CanService::sendFrame(const CanMsg &msg)
{
    // 🔥 MODE TEST → on n’envoie RIEN sur CAN1
    if (g_isTestMode) {
        LOG_VERBOSE("[TEST] CAN1 ignoré → ID=0x%X", msg.id);
        return true; // On simule un succès pour éviter les warnings
    }

    bool ok = CAN[1].send(msg);

    if (!ok)
        LOG_WARN("[CAN] Échec envoi ID=0x%X", msg.id);
    else
        LOG_VERBOSE("[CAN] TX → ID=0x%X len=%u", msg.id, msg.dlc);

    return ok;
}

// ---------------------------------------------------------------------------
// CMD_SAT_TEST_BUS (réponse à une demande de test de bus d’un satellite)
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::handleCmdTestBus(uint16_t idExp, uint8_t priorite)
{
    LOG_INFO("[CAN] TestBus reçu de %u", idExp);

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
// CMD_SAT_REQUEST_ID (réponse à une demande d’attribution d’ID d’un satellite)
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::handleCmdRequestId(uint16_t idExp, uint8_t priorite)
{
    LOG_INFO("[CAN] Demande d’ID reçue de %u", idExp);

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
            LOG_INFO("[CAN] Attribution ID=%u au satellite %u",
                     DiscoveryMaster_Settings::idNode, idExp);

            DiscoveryMaster_Settings::idNode++;
            DiscoveryMaster_Settings::writeFile();
        }
    }
    else
    {
        LOG_WARN("[CAN] Plus d’ID disponibles !");
    }
}

// ---------------------------------------------------------------------------
// COMMANDES WEB 
// ---------------------------------------------------------------------------

// Ces fonctions sont appelées par le WebHandler lors de la réception de commandes HTTP depuis l’interface web. 
// Elles construisent des messages CAN appropriés et les envoient aux satellites pour appliquer les changements demandés 
// (ex : activer/désactiver le WiFi, changer le profil de voie, etc.).
void DiscoveryMaster_CanService::sendWifiOnOff(bool on)
{
    LOG_INFO("[WEB→CAN] WIFI_ON_OFF = %s", on ? "true" : "false");

    CanMsg msg = ProtocolCAN::makeMsg(
        2, CMD_WIFI_ON_OFF, false, idMain, {uint8_t(on ? 1 : 0)});
    sendFrame(msg); 
}

// Cette fonction envoie une commande CAN pour activer ou désactiver le mode Discovery sur les satellites.
// Elle est appelée lorsque l’utilisateur clique sur le bouton correspondant dans l’interface web.
void DiscoveryMaster_CanService::sendDiscoveryOnOff(bool on)
{
    LOG_INFO("[WEB→CAN] DISCOVERY_ON_OFF = %s", on ? "true" : "false");

    CanMsg msg = ProtocolCAN::makeMsg(
        2, CMD_DISCOVERY_ON_OFF, false, idMain, {uint8_t(on ? 1 : 0)});
    sendFrame(msg);
}

// Cette fonction envoie une commande CAN pour demander à tous les satellites de sauvegarder leurs paramètres persistants (settings.json).
// Elle est appelée lorsque l’utilisateur clique sur le bouton "Save All" dans l’interface web.
void DiscoveryMaster_CanService::sendSaveAll()
{
    LOG_INFO("[WEB→CAN] SAVE_ALL");

    CanMsg msg = ProtocolCAN::makeMsg(
        2, CMD_SAVE_ALL, false, idMain, {});
    sendFrame(msg);
}

// Cette fonction envoie une commande CAN pour demander à tous les satellites de redémarrer.
// Elle est appelée lorsque l’utilisateur clique sur le bouton "Restart All" dans l’interface web.
void DiscoveryMaster_CanService::sendRestartAll()
{
    LOG_INFO("[WEB→CAN] RESTART_ALL");

    CanMsg msg = ProtocolCAN::makeMsg(
        2, CMD_RESTART_ALL, false, idMain, {});
    sendFrame(msg);
}

// Cette fonction envoie une commande CAN pour changer le profil de voie actif sur les satellites.
// Elle est appelée lorsque l’utilisateur sélectionne un profil dans l’interface web.
void DiscoveryMaster_CanService::sendTrackProfile(uint8_t profile)
{
    LOG_INFO("[WEB→CAN] SET_PROFILE = %u", profile);

    CanMsg msg = ProtocolCAN::makeMsg(
        2, CMD_SET_PROFILE, false, idMain, {uint8_t(profile)});
    sendFrame(msg);
}
