/*
DiscoveryMaster_CanService.cpp / .h

🎯 Rôle
Driver CAN du module SAMain basé sur un MCP2515 (ACAN2515).
Il constitue le cœur du protocole Discovery 2026 : réception, décodage et
émission des trames Discovery au format 29 bits (Märklin étendu).
*/

#include "DiscoveryMaster_CanService.h"

extern uint16_t idMain;
extern DiscoveryMaster_SatManager satManager;

// --- ACAN2515 ne supporte pas les lambdas → ISR statique ---
static DiscoveryMaster_CanService *g_canService = nullptr;

static void canISR()
{
    if (g_canService)
        g_canService->isr();
}

static const uint32_t CAN_BITRATE = 250UL * 1000UL; // 250 Kb/s

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
// INITIALISATION
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
// ISR
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::isr()
{
    _can.isr();
}

// ---------------------------------------------------------------------------
// LOOP CAN
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::loop()
{
    CANMessage frameIn;

    if (_can.receive(frameIn))
    {
        // Supervision CAN : on note l’heure de la dernière trame
        _lastRxTime = millis();
        _canOK = true;

        // Stockage pour le Watchdog
        lastFrame = frameIn;
        hasNewFrame = true;

#ifdef DEBUG
        debug.println("recu");
#endif
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

    if (age > timeoutMs)
    {
        _canOK = false;
    }
    else
    {
        _canOK = true;
    }

    return _canOK;
}

uint32_t DiscoveryMaster_CanService::lastRxAgeMs() const
{
    return millis() - _lastRxTime;
}

// ---------------------------------------------------------------------------
// PERMET AU WATCHDOG DE LIRE LA DERNIÈRE TRAME
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
// ENVOI TRAME (API PUBLIQUE POUR WATCHDOG & AUTRES)
// ---------------------------------------------------------------------------
bool DiscoveryMaster_CanService::sendMessage(const CANMessage &msg)
{
    CANMessage copy = msg;  // tryToSend ne prend pas un const&
    return sendFrame(copy); // sendFrame reste privé
}

// ---------------------------------------------------------------------------
// DÉCODAGE DES TRAMES
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::handleFrame(const CANMessage &frameIn)
{
    const uint8_t priorite = (frameIn.id & 0x1E000000) >> 25;
    const uint8_t cmde = (frameIn.id & 0x1FE0000) >> 17;
    const bool resp = (frameIn.id & 0x10000) >> 16;
    const uint16_t idExp = frameIn.id & 0xFFFF;

#ifdef DEBUG
    debug.printf("Reception du satellite : %d\n", idExp);
    debug.printf("commande : 0x%0X\n", cmde);
#endif

    if (resp)
        return;

    switch (cmde)
    {
    case 0xB2:
        handleCmdTestBus(idExp, priorite);
        break;

    case 0xB4:
        handleCmdRequestId(idExp, priorite);
        break;

    case 0xB0: // Heartbeat satellite
        satManager.updateHeartbeat(idExp);
        break;

    default:
        break;
    }
}

// ---------------------------------------------------------------------------
// ENVOI TRAME BAS NIVEAU (PRIVÉ)
// ---------------------------------------------------------------------------
bool DiscoveryMaster_CanService::sendFrame(CANMessage &frameOut)
{
    return _can.tryToSend(frameOut);
}

// ---------------------------------------------------------------------------
// CONSTRUCTION ID CAN
// ---------------------------------------------------------------------------
uint32_t DiscoveryMaster_CanService::buildBaseId(uint8_t priorite, uint16_t cmd, bool respFlag, uint16_t idExp)
{
    uint32_t id = 0;
    id |= (uint32_t(priorite) & 0x0F) << 25;
    id |= (uint32_t(cmd) & 0xFF) << 17;
    id |= (uint32_t(respFlag) & 0x01) << 16;
    id |= (uint32_t(idExp) & 0xFFFF);
    return id;
}

// ---------------------------------------------------------------------------
// CMD 0xB2 — TEST BUS
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::handleCmdTestBus(uint16_t idExp, uint8_t priorite)
{
#ifdef DEBUG
    debug.print("Req->Test du bus CAN\n");
#endif

    CANMessage frameOut;
    frameOut.id = buildBaseId(priorite, 0xB3, true, idExp);
    frameOut.ext = true;
    frameOut.len = 1;
    frameOut.data[0] = 1;

    sendFrame(frameOut);

    satManager.addOrUpdate(idExp);
}

// ---------------------------------------------------------------------------
// CMD 0xB4 — DEMANDE D’IDENTIFIANT
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::handleCmdRequestId(uint16_t idExp, uint8_t priorite)
{
    if (DiscoveryMaster_Settings::idNode < 253)
    {
#ifdef DEBUG
        debug.print("Req->Demande d'identifiant\n");
#endif
        CANMessage frameOut;
        frameOut.id = buildBaseId(priorite, 0xB5, true, idExp);
        frameOut.ext = true;
        frameOut.len = 1;
        frameOut.data[0] = DiscoveryMaster_Settings::idNode;

        if (sendFrame(frameOut))
        {
#ifdef DEBUG
            debug.printf("Send->Identifiant satellite : %d\n\n", DiscoveryMaster_Settings::idNode);
#endif
            DiscoveryMaster_Settings::idNode++;
            DiscoveryMaster_Settings::writeFile();
        }
    }
    else
    {
#ifdef DEBUG
        debug.printf("Taille maxi des identifiants atteinte : %d\n\n", DiscoveryMaster_Settings::idNode);
#endif
    }
}

// ---------------------------------------------------------------------------
// COMMANDES WEB
// ---------------------------------------------------------------------------
void DiscoveryMaster_CanService::sendWifiOnOff(bool on)
{
    CANMessage frame;
    frame.id = buildBaseId(2, 0xBD, false, idMain);
    frame.ext = true;
    frame.len = 1;
    frame.data[0] = on ? 1 : 0;
    sendFrame(frame);
}

void DiscoveryMaster_CanService::sendDiscoveryOnOff(bool on)
{
    CANMessage frame;
    frame.id = buildBaseId(2, 0xBE, false, idMain);
    frame.ext = true;
    frame.len = 1;
    frame.data[0] = on ? 1 : 0;
    sendFrame(frame);
}

void DiscoveryMaster_CanService::sendSaveAll()
{
    CANMessage frame;
    frame.id = buildBaseId(2, 0xBF, false, idMain);
    frame.ext = true;
    frame.len = 0;
    sendFrame(frame);
}

void DiscoveryMaster_CanService::sendRestartAll()
{
    CANMessage frame;
    frame.id = buildBaseId(2, 0xBC, false, idMain);
    frame.ext = true;
    frame.len = 0;
    sendFrame(frame);
}
