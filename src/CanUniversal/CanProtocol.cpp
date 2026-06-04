#include "CanProtocol.h"

// ---------------------------------------------------------------------------
// ENCODEUR ID 29 bits
// ---------------------------------------------------------------------------
uint32_t CanProtocol::make(uint8_t prio, uint8_t cmd, bool resp, uint16_t src) {
    uint32_t id = 0;

    id |= (uint32_t(prio) & PROTOCOLCAN_MASK_PRIORITY)  << PROTOCOLCAN_SHIFT_PRIORITY;
    id |= (uint32_t(cmd)  & PROTOCOLCAN_MASK_COMMAND)   << PROTOCOLCAN_SHIFT_COMMAND;
    id |= (uint32_t(resp) & PROTOCOLCAN_MASK_RESPONSE)  << PROTOCOLCAN_SHIFT_RESPONSE;
    id |= (uint32_t(src)  & PROTOCOLCAN_MASK_SOURCE);

    return id;
}

// ---------------------------------------------------------------------------
// DÉCODEUR ID 29 bits
// ---------------------------------------------------------------------------
CanProtocolFields CanProtocol::decode(uint32_t id) {
    CanProtocolFields f;

    f.prio = (id >> PROTOCOLCAN_SHIFT_PRIORITY) & PROTOCOLCAN_MASK_PRIORITY;
    f.cmd  = (id >> PROTOCOLCAN_SHIFT_COMMAND)  & PROTOCOLCAN_MASK_COMMAND;
    f.resp = (id >> PROTOCOLCAN_SHIFT_RESPONSE) & PROTOCOLCAN_MASK_RESPONSE;
    f.src  = id & PROTOCOLCAN_MASK_SOURCE;

    return f;
}

// ---------------------------------------------------------------------------
// CONSTRUCTION D’UN MESSAGE COMPLET (29 bits)
// ---------------------------------------------------------------------------
CanMsg CanProtocol::makeMsg(uint8_t prio,
                            uint8_t cmd,
                            bool resp,
                            uint16_t src,
                            std::initializer_list<uint8_t> payload) {

    uint32_t id = make(prio, cmd, resp, src);
    return CanMsg(id, payload);
}