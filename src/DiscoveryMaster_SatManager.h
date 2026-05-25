#pragma once

#include <Arduino.h>
#include "DiscoveryMaster_Satellite.h"
#include "DiscoveryMaster_Config.h"

class DiscoveryMaster_SatManager
{
public:
    DiscoveryMaster_SatManager();

    void begin();
    void loop();

    void addOrUpdate(uint16_t idSat);
    DiscoveryMaster_Satellite *getById(uint16_t idSat);
    uint8_t count() const;

private:
    DiscoveryMaster_Satellite _sats[NB_SAT];
};
