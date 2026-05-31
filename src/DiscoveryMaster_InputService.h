#pragma once
#include <Arduino.h>

class DiscoveryMaster_InputService
{
public:
    void begin();
    void loop();

private:
    bool _prevBtn = true;
};
