#include "ProtocolCAN.h"
#include "CanID.h"

// ---------------------------------------------------------------------------
// ENCODEUR ID 29 bits (utilise CanID)
// ---------------------------------------------------------------------------
uint32_t ProtocolCAN::make(uint8_t prio, uint8_t cmd, bool resp, uint16_t src)
{
    return CanID::make29(prio, cmd, resp, src);
}

// ---------------------------------------------------------------------------
// DÉCODEUR ID 29 bits (utilise CanID)
// ---------------------------------------------------------------------------
ProtocolCANFields ProtocolCAN::decode(uint32_t id)
{
    ProtocolCANFields f;

    f.prio = CanID::prio(id);
    f.cmd = CanID::cmde(id);
    f.resp = CanID::resp(id);
    f.src = CanID::nodeId(id);

    return f;
}

// ---------------------------------------------------------------------------
// HELPERS 11 bits (inchangés)
// ---------------------------------------------------------------------------
bool ProtocolCAN::isHeartbeat(uint32_t id)
{
    return id == PROTOCOLCAN_ID_HEARTBEAT;
}

bool ProtocolCAN::isStop(uint32_t id)
{
    return id == PROTOCOLCAN_ID_STOP;
}

bool ProtocolCAN::isClearStop(uint32_t id)
{
    return id == PROTOCOLCAN_ID_CLEAR_STOP;
}

// ---------------------------------------------------------------------------
// CONSTRUCTION D’UN MESSAGE COMPLET (29 bits)
// ---------------------------------------------------------------------------
CanMsg ProtocolCAN::makeMsg(uint8_t prio,
                            uint8_t cmd,
                            bool resp,
                            uint16_t src,
                            std::initializer_list<uint8_t> payload)
{

    uint32_t id = make(prio, cmd, resp, src);
    return CanMsg(id, payload);
}
