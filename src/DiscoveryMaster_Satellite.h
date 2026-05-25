#pragma once
#include <Arduino.h>
#include "DiscoveryMaster_Config.h"

class DiscoveryMaster_Satellite
{
private:
    uint16_t m_idNode;
    uint8_t m_ip[4];

public:
    DiscoveryMaster_Satellite();
    void begin();

    // --- IMPORTANT : rendre id() const ---
    uint16_t id() const;

    void id(uint16_t);

    static void watchDog(void *);
};
