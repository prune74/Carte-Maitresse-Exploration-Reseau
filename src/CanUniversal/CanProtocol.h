#pragma once
#include <stdint.h>
#include <initializer_list>
#include "CanUniversal/CanMsg.h"

// ---------------------------------------------------------------------------
// Champs du protocole 29 bits Discovery
// ---------------------------------------------------------------------------
#define PROTOCOLCAN_SHIFT_PRIORITY   25
#define PROTOCOLCAN_SHIFT_COMMAND    17
#define PROTOCOLCAN_SHIFT_RESPONSE   16

#define PROTOCOLCAN_MASK_PRIORITY    0x0F
#define PROTOCOLCAN_MASK_COMMAND     0xFF
#define PROTOCOLCAN_MASK_RESPONSE    0x01
#define PROTOCOLCAN_MASK_SOURCE      0xFFFF

// ---------------------------------------------------------------------------
// Structure décodée
// ---------------------------------------------------------------------------
struct CanProtocolFields {
    uint8_t  prio;
    uint8_t  cmd;
    bool     resp;
    uint16_t src;
};

class CanProtocol {
public:
    // Encodeur ID 29 bits
    static uint32_t make(uint8_t prio, uint8_t cmd, bool resp, uint16_t src);

    // Décodeur ID 29 bits
    static CanProtocolFields decode(uint32_t id);

    // Construction complète d’un message CAN (29 bits)
    static CanMsg makeMsg(uint8_t prio,
                          uint8_t cmd,
                          bool resp,
                          uint16_t src,
                          std::initializer_list<uint8_t> payload);
};
