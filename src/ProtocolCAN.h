#pragma once
#include <stdint.h>
#include <initializer_list>
#include "CanMsg.h"

// ---------------------------------------------------------------------------
// IDs 11 bits — messages globaux Master ↔ SA
// ---------------------------------------------------------------------------
#define PROTOCOLCAN_ID_HEARTBEAT 0x200
#define PROTOCOLCAN_ID_STOP 0x201
#define PROTOCOLCAN_ID_CLEAR_STOP 0x202

// ---------------------------------------------------------------------------
// Structure décodée
// ---------------------------------------------------------------------------
struct ProtocolCANFields
{
    uint8_t prio;
    uint8_t cmd;
    bool resp;
    uint16_t src;
};

class ProtocolCAN
{
public:
    // Encodeur ID 29 bits
    static uint32_t make(uint8_t prio, uint8_t cmd, bool resp, uint16_t src);

    // Décodeur ID 29 bits
    static ProtocolCANFields decode(uint32_t id);

    // Helpers 11 bits
    static bool isHeartbeat(uint32_t id);
    static bool isStop(uint32_t id);
    static bool isClearStop(uint32_t id);

    // Construction complète d’un message CAN (29 bits)
    static CanMsg makeMsg(uint8_t prio,
                          uint8_t cmd,
                          bool resp,
                          uint16_t src,
                          std::initializer_list<uint8_t> payload);
};
