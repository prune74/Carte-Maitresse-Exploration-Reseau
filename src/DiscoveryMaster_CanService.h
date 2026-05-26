#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <ACAN2515.h>

#include "DiscoveryMaster_Config.h"
#include "DiscoveryMaster_SatManager.h"
#include "DiscoveryMaster_Settings.h"

class DiscoveryMaster_CanService
{
private:
    ACAN2515 _can;
    uint8_t _csPin;
    uint8_t _intPin;

    CANMessage lastFrame;
    bool hasNewFrame;

    // --- Supervision CAN ---
    uint32_t _lastRxTime;   // millis() de la dernière trame reçue
    bool _canOK;            // état courant du bus CAN

    bool sendFrame(CANMessage &frameOut);
    void handleFrame(const CANMessage &frameIn);

    void handleCmdTestBus(uint16_t idExp, uint8_t priorite);
    void handleCmdRequestId(uint16_t idExp, uint8_t priorite);

public:
    DiscoveryMaster_CanService(uint8_t csPin, uint8_t intPin);

    bool begin();
    void isr();
    void loop();

    // Watchdog
    bool getLastFrame(CANMessage &msg);

    // API publique d’envoi
    bool sendMessage(const CANMessage &msg);

    // Construction ID Discovery
    static uint32_t buildBaseId(uint8_t priorite, uint16_t cmd, bool respFlag, uint16_t idExp);

    // Commandes Web
    void sendWifiOnOff(bool on);
    void sendDiscoveryOnOff(bool on);
    void sendSaveAll();
    void sendRestartAll();

    // --- Supervision CAN ---
    bool checkBus(uint32_t timeoutMs);      // à appeler périodiquement
    bool isCanOK() const { return _canOK; } // pour l’UI
    uint32_t lastRxAgeMs() const;           // âge de la dernière trame
};
