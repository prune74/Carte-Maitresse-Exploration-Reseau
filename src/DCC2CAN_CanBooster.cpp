#include "DCC2CAN_CanBooster.h"
#include "CanInit.h"
#include "CanMsg.h"
#include "CanBus.h"
#include "Debug.h"   // 🔥 Ajout du système de logs

// -----------------------------------------------------------------------------
// Initialisation du CAN Booster (CAN0)
// -----------------------------------------------------------------------------
void CanBooster_begin() {

    // Rien à faire ici : l’application appelle CanInit::begin(CAN_CONFIG)
    LOG_INFO("CanBooster → CAN0 prêt (via CanInit)");
}

// -----------------------------------------------------------------------------
// Envoi du bit DCC (bit + phase)
// -----------------------------------------------------------------------------
bool CanBooster_sendDccBit(uint8_t bit, uint8_t phase) {

    // ID 11 bits → cast explicite obligatoire
    CanMsg msg((uint16_t)DCCB_CAN_ID_DCC_BIT, { bit, phase });

    // Debug très détaillé (optionnel)
    LOG_VERBOSE("DCC2CAN → bit=%u phase=%u", bit, phase);

    // Nouveau système : accès via CanBus::bus(index)
    bool ok = CanBus::bus(0).send(msg);

    if (!ok) {
        LOG_WARN("DCC2CAN → Échec d’envoi du bit DCC !");
    }

    return ok;
}
