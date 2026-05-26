#pragma once
#include <Arduino.h>
#include "DiscoveryMaster_Config.h"
#include "DiscoveryMaster_Satellite.h"

class DiscoveryMaster_SatManager
{
private:
    DiscoveryMaster_Satellite _sats[NB_SAT];

public:
    DiscoveryMaster_SatManager();

    void begin();
    void loop();

    void addOrUpdate(uint16_t idSat);
    void updateHeartbeat(uint16_t idSat);
    void checkTimeouts(uint32_t timeoutMs);

    DiscoveryMaster_Satellite *getById(uint16_t idSat);
    uint8_t count() const;
    const DiscoveryMaster_Satellite* getAll() const { return _sats; }
};
