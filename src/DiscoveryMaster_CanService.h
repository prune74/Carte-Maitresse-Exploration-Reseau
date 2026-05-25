#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <ACAN2515.h>
#include "DiscoveryMaster_Config.h"
#include "DiscoveryMaster_Satellite.h"
#include "DiscoveryMaster_Settings.h"
#include "DiscoveryMaster_SatManager.h"

class DiscoveryMaster_CanService
{
public:
    DiscoveryMaster_CanService(uint8_t csPin, uint8_t intPin);

    bool begin();
    void loop();
    void isr();

    // Commandes Web
    void sendWifiOnOff(bool on);
    void sendDiscoveryOnOff(bool on);
    void sendSaveAll();
    void sendRestartAll();

    // Watchdog Discovery 2026
    bool getLastFrame(CANMessage &msg);
    bool sendMessage(const CANMessage &msg);

private:
    ACAN2515 _can;
    uint8_t _csPin;
    uint8_t _intPin;

    // Buffer pour Watchdog
    CANMessage lastFrame;
    bool hasNewFrame = false;

    // Protocole Discovery
    void handleFrame(const CANMessage &frameIn);
    bool sendFrame(CANMessage &frameOut);

    void handleCmdTestBus(uint16_t idExp, uint8_t priorite);
    void handleCmdRequestId(uint16_t idExp, uint8_t priorite);

    uint32_t buildBaseId(uint8_t priorite, uint16_t cmd, bool respFlag, uint16_t idExp);
};
