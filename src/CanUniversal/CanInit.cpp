#include "CanInit.h"
#include "CanConfig.h"

// -----------------------------------------------------------------------------
// Drivers ACAN globaux
// -----------------------------------------------------------------------------
// ACAN_ESP32 : driver statique ACAN_ESP32::can
// ACAN2515   : doit être instancié ici
// -----------------------------------------------------------------------------
ACAN2515 can2515(CAN1_CS_PIN, SPI, CAN1_INT_PIN);

// -----------------------------------------------------------------------------
// Tableau global de bus CAN
// -----------------------------------------------------------------------------
CanBus CAN[CAN_COUNT] = {
    CanBus(&ACAN_ESP32::can), // CAN0 = ESP32 interne
    CanBus(&can2515)          // CAN1 = MCP2515 externe
    // Si CAN_COUNT > 2, ajouter ici d'autres drivers
};


// -----------------------------------------------------------------------------
// ISR MCP2515
// -----------------------------------------------------------------------------
static void canISR() {
    can2515.isr();
}


// -----------------------------------------------------------------------------
// Initialisation globale CAN
// -----------------------------------------------------------------------------
bool CanUniversal_begin() {

    // =========================================================================
    // CAN0 : ESP32 interne (optionnel)
    // =========================================================================
    if (CAN0_ENABLED) {

        ACAN_ESP32_Settings settings(CAN0_SPEED);
        settings.mTxPin = CAN0_TX_PIN;
        settings.mRxPin = CAN0_RX_PIN;

        uint32_t err = ACAN_ESP32::can.begin(settings);

        if (err != 0) {
            Serial.printf("[CAN0] Erreur ACAN_ESP32 : 0x%X → ignoré\n", err);
        } else {
            Serial.println("[CAN0] OK (ESP32 interne)");
        }

    } else {
        Serial.println("[CAN0] désactivé dans CanConfig.h");
    }


    // =========================================================================
    // CAN1 : MCP2515 externe (optionnel)
    // =========================================================================
    if (CAN1_ENABLED) {

        ACAN2515Settings settings(
            CAN1_QUARTZ,
            CAN1_SPEED,
            CAN1_TOLERANCE
        );

        uint32_t err = can2515.begin(settings, canISR);

        if (err != 0) {
            Serial.printf("[CAN1] MCP2515 non détecté (err=0x%X) → ignoré\n", err);
        } else {
            Serial.println("[CAN1] OK (MCP2515 externe)");
        }

    } else {
        Serial.println("[CAN1] désactivé dans CanConfig.h");
    }

    return true;
}

