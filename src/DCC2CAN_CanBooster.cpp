#include "DCC2CAN_CanBooster.h"
#include "CanInit.h"
#include "CanMsg.h"
#include "CanBus.h"

// -----------------------------------------------------------------------------
// Initialisation du CAN Booster (CAN0)
// -----------------------------------------------------------------------------
void CanBooster_begin() {
    // Rien à faire ici : l’application appelle CanInit::begin(CAN_CONFIG)
    Serial.println("DCC2CAN → CAN0 prêt (via CanInit)");
}

// -----------------------------------------------------------------------------
// Envoi du bit DCC (bit + phase)
// -----------------------------------------------------------------------------
bool CanBooster_sendDccBit(uint8_t bit, uint8_t phase) {

    // ID 11 bits → cast explicite obligatoire
    CanMsg msg((uint16_t)DCCB_CAN_ID_DCC_BIT, { bit, phase });

    // Nouveau système : accès via CanBus::bus(index)
    return CanBus::bus(0).send(msg);
}
