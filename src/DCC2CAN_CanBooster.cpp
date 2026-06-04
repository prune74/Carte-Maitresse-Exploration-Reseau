#include "DCC2CAN_CanBooster.h"
#include "CanUniversal/CanInit.h"
#include "CanUniversal/CanMsg.h"
#include "CanUniversal/CanBus.h"

// -----------------------------------------------------------------------------
// Initialisation du CAN Booster (CAN0)
// -----------------------------------------------------------------------------
void CanBooster_begin() {
    // Rien à initialiser ici : CanUniversal_begin() s’en charge
    Serial.println("DCC2CAN → CAN0 prêt (via CanUniversal)");
}

// -----------------------------------------------------------------------------
// Envoi du bit DCC (bit + phase)
// -----------------------------------------------------------------------------
bool CanBooster_sendDccBit(uint8_t bit, uint8_t phase) {

    // ID 11 bits → cast explicite obligatoire
    CanMsg msg((uint16_t)DCCB_CAN_ID_DCC_BIT, { bit, phase });

    return CAN[0].send(msg);
}
