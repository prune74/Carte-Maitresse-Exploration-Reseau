#include "ProtocolCAN.h"
#include "Protocol.h"
#include "CanID.h"
#include "Debug.h"

// ---------------------------------------------------------------------------
// ENCODEUR ID 29 bits (utilise CanID)
// ---------------------------------------------------------------------------
uint32_t ProtocolCAN::make(uint8_t prio, uint8_t cmd, bool resp, uint16_t src)
{
    uint32_t id = CanID::make29(prio, cmd, resp, src);
    LOG_VERBOSE("ProtocolCAN → make(prio=%u cmd=0x%X resp=%u src=%u) → 0x%08X",
                prio, cmd, resp, src, id);
    return id;
}

// ---------------------------------------------------------------------------
// DÉCODEUR ID 29 bits (utilise CanID)
// ---------------------------------------------------------------------------
ProtocolCANFields ProtocolCAN::decode(uint32_t id)
{
    ProtocolCANFields f;

    f.prio = CanID::prio(id);
    f.cmd  = CanID::cmde(id);
    f.resp = CanID::resp(id);
    f.src  = CanID::nodeId(id);

    LOG_VERBOSE("ProtocolCAN → decode(0x%08X) → prio=%u cmd=0x%X resp=%u src=%u",
                id, f.prio, f.cmd, f.resp, f.src);

    return f;
}

// ---------------------------------------------------------------------------
// HELPERS 11 bits (inchangés)
// ---------------------------------------------------------------------------
bool ProtocolCAN::isHeartbeat(uint32_t id)
{
    return id == uint16_t(Cmd_Global11::HEARTBEAT);
}

bool ProtocolCAN::isStop(uint32_t id)
{
    return id == uint16_t(Cmd_Global11::EMERGENCY_STOP);
}

bool ProtocolCAN::isClearStop(uint32_t id)
{
    return id == uint16_t(Cmd_Global11::CLEAR_STOP);
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

    LOG_VERBOSE("ProtocolCAN → makeMsg(ID=0x%08X, len=%u)", id, payload.size());

    return CanMsg(id, payload);
}
