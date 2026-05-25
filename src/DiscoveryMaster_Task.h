#pragma once
#include "DiscoveryMaster_Config.h"

class DiscoveryMaster_Task
{
private:
public:
  DiscoveryMaster_Task() = delete;
  static void begin();
  static void task1(void *);
};
