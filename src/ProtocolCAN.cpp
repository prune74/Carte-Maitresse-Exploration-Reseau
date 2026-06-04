#include "ProtocolCAN.h"
#include "DiscoveryMaster_Config.h"

// ---------------------------------------------------------------------------
// HELPERS 11 bits
// ---------------------------------------------------------------------------
bool ProtocolCAN::isHeartbeat(uint32_t id) {
    return id == PROTOCOLCAN_ID_HEARTBEAT;
}

bool ProtocolCAN::isStop(uint32_t id) {
    return id == PROTOCOLCAN_ID_STOP;
}

bool ProtocolCAN::isClearStop(uint32_t id) {
    return id == PROTOCOLCAN_ID_CLEAR_STOP;
}