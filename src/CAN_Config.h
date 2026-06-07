#pragma once
#include "CanConfig.h"

/**
 * ============================================================================
 *  MasterConfig
 * ============================================================================
 *  Fournit la configuration CAN pour la carte Discovery Master.
 *
 *  La carte Master possède DEUX bus CAN :
 *
 *      - CAN0 : bus interne ESP32 (TWAI) → utilisé pour le Booster
 *      - CAN1 : bus externe MCP2515 → utilisé pour le CanService Discovery
 *
 *  Cette classe implémente l’interface CanConfigProvider, ce qui permet à
 *  CanInit::begin() d’interroger dynamiquement :
 *
 *      - le nombre de bus CAN disponibles
 *      - la configuration détaillée de chaque bus
 *
 *  Avantages :
 *    - configuration centralisée et claire
 *    - aucun code matériel dans la bibliothèque CanUniversal
 *    - facile à adapter si la carte évolue
 * ============================================================================
 */
class MasterConfig : public CanConfigProvider {
public:

    /**
     * La carte Master utilise 2 bus CAN :
     *   - CAN0 : ESP32 interne
     *   - CAN1 : MCP2515 externe
     */
    uint8_t busCount() const override { return 2; }

    /**
     * Retourne la configuration du bus CAN demandé.
     *
     * index = 0 → bus interne ESP32 (TWAI)
     * index = 1 → bus MCP2515 externe
     */
    const CanBusConfig& bus(uint8_t index) const override {
        static CanBusConfig cfg[2] = {

            // -----------------------------------------------------------------
            // CAN0 : ESP32 interne (TWAI) → Booster
            // -----------------------------------------------------------------
            {
                true,          // enabled
                500000,        // vitesse CAN Booster
                GPIO_NUM_4,    // TX ESP32
                GPIO_NUM_5,    // RX ESP32

                // Pas de MCP2515 → toutes les pins SPI sont NC
                GPIO_NUM_NC,   // cs_pin
                GPIO_NUM_NC,   // int_pin
                GPIO_NUM_NC,   // sck_pin
                GPIO_NUM_NC,   // mosi_pin
                GPIO_NUM_NC,   // miso_pin

                0,             // quartz (non utilisé)
                0              // tolerance (non utilisé)
            },

            // -----------------------------------------------------------------
            // CAN1 : MCP2515 externe → CanService Discovery
            // -----------------------------------------------------------------
            {
                true,           // enabled
                250000,         // vitesse CAN Discovery
                GPIO_NUM_NC,    // tx_pin (NC → bus interne non utilisé)
                GPIO_NUM_NC,    // rx_pin

                GPIO_NUM_13,    // cs_pin
                GPIO_NUM_14,    // int_pin
                GPIO_NUM_18,    // sck_pin
                GPIO_NUM_23,    // mosi_pin
                GPIO_NUM_19,    // miso_pin

                8000000,        // quartz MCP2515 (8 MHz)
                50              // tolérance ACAN2515 (%)
            }
        };

        return cfg[index];
    }
};

/**
 * Objet global utilisé par l’application pour initialiser le CAN :
 *
 *      CanInit::begin(MASTER_CAN_CONFIG);
 *
 * Il doit être défini dans MasterConfig.cpp.
 */
extern MasterConfig MASTER_CAN_CONFIG;
