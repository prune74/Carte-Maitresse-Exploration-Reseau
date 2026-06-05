#pragma once
#include "CanMsg.h"
#include "CanBus.h"

class DiscoveryMaster_CanService
{
public:
    DiscoveryMaster_CanService();

    bool begin();
    void loop();

    bool checkBus(uint32_t timeoutMs = 500);
    uint32_t lastRxAgeMs() const;
    bool isCanOK() const { return _canOK; }

    bool getLastFrame(CanMsg &msg);
    bool sendMessage(const CanMsg &msg);

    // Commandes satellites
    void handleCmdTestBus(uint16_t idExp, uint8_t priorite);
    void handleCmdRequestId(uint16_t idExp, uint8_t priorite);

    // Commandes Web
    void sendWifiOnOff(bool on);
    void sendDiscoveryOnOff(bool on);
    void sendSaveAll();
    void sendRestartAll();
    void sendTrackProfile(uint8_t profile);

private:
    void handleFrame(const CanMsg &msg);
    bool sendFrame(const CanMsg &msg);

private:
    uint32_t _lastRxTime;
    bool _canOK;

    bool hasNewFrame;
    CanMsg lastFrame;
};
