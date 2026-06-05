#pragma once
#include "CanConfig.h"

/**
 * ============================================================================
 *  MasterConfig
 * ============================================================================
 *  Fournit la configuration CAN pour la carte Discovery Master.
 *
 *  Cette classe implémente l’interface CanConfigProvider, ce qui permet à
 *  CanInit::begin() d’interroger dynamiquement :
 *
 *      - le nombre de bus CAN disponibles
 *      - la configuration détaillée de chaque bus
 *
 *  Ici, la carte Master utilise un seul bus CAN externe basé sur un MCP2515.
 *  Le bus interne ESP32 n’est pas utilisé.
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
     * Retourne le nombre total de bus CAN utilisés par la carte Master.
     * Ici : un seul bus (MCP2515).
     */
    uint8_t busCount() const override { return 1; }

    /**
     * Retourne la configuration du bus CAN demandé.
     *
     * index = 0 → bus MCP2515 externe
     *
     * Détails :
     *   - vitesse : 250 kbps (réseau Discovery Master)
     *   - cs/int : broches dédiées au MCP2515
     *   - SPI    : SCK / MOSI / MISO
     *   - quartz : 8 MHz
     *   - tolérance : 50% (ACAN2515)
     */
    const CanBusConfig& bus(uint8_t index) const override {
        static CanBusConfig cfg = {
            true,           // enabled
            250000,         // vitesse CAN (250 kbps)
            GPIO_NUM_NC,    // tx_pin  (NC → bus interne non utilisé)
            GPIO_NUM_NC,    // rx_pin  (NC → bus interne non utilisé)

            // MCP2515 (bus SPI)
            GPIO_NUM_13,    // cs_pin
            GPIO_NUM_14,    // int_pin
            GPIO_NUM_18,    // sck_pin
            GPIO_NUM_23,    // mosi_pin
            GPIO_NUM_19,    // miso_pin

            8000000,        // quartz MCP2515 (8 MHz)
            50              // tolérance ACAN2515 (%)
        };
        return cfg;
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
