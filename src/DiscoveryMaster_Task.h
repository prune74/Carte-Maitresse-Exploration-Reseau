#pragma once
#include <Arduino.h>
#include "DiscoveryMaster_SatManager.h"
#include "DiscoveryMaster_CanService.h"
#include "DiscoveryMaster_WebHandler.h"

class DiscoveryMaster_Task
{
public:
    static void begin();
    static void taskLoop(void *pvParameters);
};
