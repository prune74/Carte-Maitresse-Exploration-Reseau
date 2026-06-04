#pragma once
#include <stdint.h>
#include <initializer_list>
#include "CanUniversal/CanMsg.h"

class ProtocolCAN {
public:
    // Helpers 11 bits
    static bool isHeartbeat(uint32_t id);
    static bool isStop(uint32_t id);
    static bool isClearStop(uint32_t id);
};
