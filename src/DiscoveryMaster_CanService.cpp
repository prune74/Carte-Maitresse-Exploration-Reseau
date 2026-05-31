/*
 * DiscoveryMaster_CanService.cpp / .h
 *
 * 🎯 Rôle
 * Driver CAN du module SAMain basé sur un MCP2515 (ACAN2515).
 * Il constitue le cœur du protocole Discovery 2026 : réception, décodage et
 * émission des trames Discovery au format 29 bits (Märklin étendu).
 *
 * 📌 Fonctionnement général
 * - Initialisation du MCP2515 via ACAN2515
 * - Réception des trames Discovery (29 bits)
 * - Décodage des champs :
 *      • priorité (4 bits)
 *      • commande Discovery (8 bits)
 *      • flag réponse (1 bit)
 *      • ID émetteur (16 bits)
 * - Gestion des commandes satellites :
 *      • Test bus
 *      • Demande d’identifiant
 *      • Heartbeat
 * - Gestion des commandes système :
 *      • STOP global (0x201)
 *      • CLEAR STOP global (0x202)
 *      • LED STOP / LED RUN
 *
 * 📌 Particularités
 * - ACAN2515 ne supporte pas les lambdas → ISR statique obligatoire
 * - Supervision CAN intégrée (lastRxTime, canOK)
 * - Stockage de la dernière trame pour le Watchdog
 */

#include "DiscoveryMaster_CanService.h"
#include "Discovery_Protocol.h"
#include "DiscoveryMaster_Pins.h"   // LED STOP / RUN

extern uint16_t idMain;
extern DiscoveryMaster_SatManager satManager;

// ---------------------------------------------------------------------------
// ACAN2515 : ISR statique obligatoire
// ---------------------------------------------------------------------------
static DiscoveryMaster_CanService *g_canService = nullptr;

static void canISR()
{
    if (g_canService)
        g_canService->isr();
}

static const uint32_t CAN_BITRATE = 250UL * 1000UL; // 250 Kb/s Discovery

// ---------------------------------------------------------------------------
// CONSTRUCTEUR
// ---------------------------------------------------------------------------
DiscoveryMaster_CanService::DiscoveryMaster_CanService(uint8_t csPin, uint8_t intPin)
    : _can(csPin, SPI, intPin), _csPin(csPin), _intPin(intPin)
{
    g_canService = this;
    hasNewFrame = false;

    // Supervision CAN
    _lastRxTime = millis();
    _canOK = false; // au démarrage, on attend une première trame
}

// ---------------------------------------------------------------------------
// INITIALISATION DU MCP2515
// ---------------------------------------------------------------------------
bool DiscoveryMaster_CanService::begin()
{
    Serial.print("Configure MCP2515 CAN Service ");

    SPI.begin(18, 19, 23); // SCK, MISO, MOSI

    ACAN2515Settings settings(16UL * 1000UL * 1000UL, CAN_BITRATE);

    uint32_t errorCode = _can.begin(settings, canISR);

    if (errorCode == 0)
    {
        Serial.println("ok !");
        return true;
    }
    else
    {
        Serial.printf("error 0x%x\n", errorCode);
        return false;
    }
}

// ---------------------------------------------------------------------------
// ISR CAN
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::isr()
{
    _can.isr();
}

// ---------------------------------------------------------------------------
// LOOP CAN : réception + supervision
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::loop()
{
    CANMessage frameIn;

    if (_can.receive(frameIn))
    {
        _lastRxTime = millis();
        _canOK = true;

        lastFrame = frameIn;
        hasNewFrame = true;

        handleFrame(frameIn);
    }
}

// ---------------------------------------------------------------------------
// SUPERVISION CAN
// ---------------------------------------------------------------------------
bool DiscoveryMaster_CanService::checkBus(uint32_t timeoutMs)
{
    uint32_t now = millis();
    uint32_t age = now - _lastRxTime;

    _canOK = (age <= timeoutMs);
    return _canOK;
}

uint32_t DiscoveryMaster_CanService::lastRxAgeMs() const
{
    return millis() - _lastRxTime;
}

// ---------------------------------------------------------------------------
// LECTURE DERNIÈRE TRAME (Watchdog)
// ---------------------------------------------------------------------------
bool DiscoveryMaster_CanService::getLastFrame(CANMessage &msg)
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
// API PUBLIQUE : ENVOI TRAME
// ---------------------------------------------------------------------------
bool DiscoveryMaster_CanService::sendMessage(const CANMessage &msg)
{
    CANMessage copy = msg;
    return sendFrame(copy);
}

// ---------------------------------------------------------------------------
// DÉCODAGE DES TRAMES DISCOVERY
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::handleFrame(const CANMessage &frameIn)
{
    const uint8_t priorite = (frameIn.id & 0x1E000000) >> 25;
    const uint8_t cmde     = (frameIn.id & 0x1FE0000)  >> 17;
    const bool resp        = (frameIn.id & 0x10000)    >> 16;
    const uint16_t idExp   = frameIn.id & 0xFFFF;

    // -----------------------------------------------------------------------
    // 🟥 STOP GLOBAL (0x201)
    // -----------------------------------------------------------------------
    if (frameIn.id == DISCOVERY_CAN_ID_EMERGENCY_STOP)
    {
        digitalWrite(PIN_LED_STOP, HIGH);  // LED rouge ON
        Serial.println("[MASTER][CAN] STOP global reçu");
        return;
    }

    // -----------------------------------------------------------------------
    // 🟩 CLEAR STOP GLOBAL (0x202)
    // -----------------------------------------------------------------------
    if (frameIn.id == DISCOVERY_CAN_ID_CLEAR_STOP)
    {
        digitalWrite(PIN_LED_STOP, LOW);   // LED rouge OFF
        Serial.println("[MASTER][CAN] CLEAR STOP reçu");
        return;
    }

    // -----------------------------------------------------------------------
    // Ignorer les réponses
    // -----------------------------------------------------------------------
    if (resp)
        return;

    // -----------------------------------------------------------------------
    // Commandes satellites
    // -----------------------------------------------------------------------
    switch (cmde)
    {
    case CMD_SAT_TEST_BUS:
        handleCmdTestBus(idExp, priorite);
        break;

    case CMD_SAT_REQUEST_ID:
        handleCmdRequestId(idExp, priorite);
        break;

    case CMD_SAT_HEARTBEAT:
        satManager.updateHeartbeat(idExp);
        break;

    default:
        break;
    }
}

// ---------------------------------------------------------------------------
// ENVOI BAS NIVEAU
// ---------------------------------------------------------------------------
bool DiscoveryMaster_CanService::sendFrame(CANMessage &frameOut)
{
    return _can.tryToSend(frameOut);
}

// ---------------------------------------------------------------------------
// CONSTRUCTION ID CAN DISCOVERY
// ---------------------------------------------------------------------------
uint32_t DiscoveryMaster_CanService::buildBaseId(uint8_t priorite, uint16_t cmd, bool respFlag, uint16_t idExp)
{
    uint32_t id = 0;
    id |= (uint32_t(priorite) & 0x0F) << 25;
    id |= (uint32_t(cmd)      & 0xFF) << 17;
    id |= (uint32_t(respFlag) & 0x01) << 16;
    id |= (uint32_t(idExp)    & 0xFFFF);
    return id;
}

// ---------------------------------------------------------------------------
// CMD_SAT_TEST_BUS
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::handleCmdTestBus(uint16_t idExp, uint8_t priorite)
{
    CANMessage frameOut;
    frameOut.id = buildBaseId(priorite, CMD_SAT_TEST_BUS_REPLY, true, idExp);
    frameOut.ext = true;
    frameOut.len = 1;
    frameOut.data[0] = 1;

    sendFrame(frameOut);
    satManager.addOrUpdate(idExp);
}

// ---------------------------------------------------------------------------
// CMD_SAT_REQUEST_ID
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::handleCmdRequestId(uint16_t idExp, uint8_t priorite)
{
    if (DiscoveryMaster_Settings::idNode < 253)
    {
        CANMessage frameOut;
        frameOut.id = buildBaseId(priorite, CMD_SAT_REQUEST_ID_REPLY, true, idExp);
        frameOut.ext = true;
        frameOut.len = 1;
        frameOut.data[0] = DiscoveryMaster_Settings::idNode;

        if (sendFrame(frameOut))
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
    CANMessage frame;
    frame.id = buildBaseId(2, CMD_WIFI_ON_OFF, false, idMain);
    frame.ext = true;
    frame.len = 1;
    frame.data[0] = on ? 1 : 0;
    sendFrame(frame);
}

void DiscoveryMaster_CanService::sendDiscoveryOnOff(bool on)
{
    CANMessage frame;
    frame.id = buildBaseId(2, CMD_DISCOVERY_ON_OFF, false, idMain);
    frame.ext = true;
    frame.len = 1;
    frame.data[0] = on ? 1 : 0;
    sendFrame(frame);
}

void DiscoveryMaster_CanService::sendSaveAll()
{
    CANMessage frame;
    frame.id = buildBaseId(2, CMD_SAVE_ALL, false, idMain);
    frame.ext = true;
    frame.len = 0;
    sendFrame(frame);
}

void DiscoveryMaster_CanService::sendRestartAll()
{
    CANMessage frame;
    frame.id = buildBaseId(2, CMD_RESTART_ALL, false, idMain);
    frame.ext = true;
    frame.len = 0;
    sendFrame(frame);
}

// ---------------------------------------------------------------------------
// CMD_SET_PROFILE — Profil voie (N / HO)
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::sendTrackProfile(uint8_t profile)
{
    CANMessage frame;
    frame.id = buildBaseId(2, CMD_SET_PROFILE, false, idMain);
    frame.ext = true;
    frame.len = 1;
    frame.data[0] = profile;

    sendFrame(frame);
}
