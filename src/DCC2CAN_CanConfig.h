#pragma once
#include "CanConfig.h"

/**
 * ============================================================================
 *  Dcc2CanConfig
 * ============================================================================
 *  Fournit la configuration CAN pour le module DCC2CAN.
 *
 *  Ce module utilise exclusivement :
 *      → le bus CAN interne de l’ESP32 (TWAI)
 *
 *  Le rôle de cette classe :
 *    - indiquer à CanInit combien de bus CAN sont utilisés
 *    - fournir la configuration détaillée de chaque bus
 *    - permettre une initialisation totalement générique via CanInit::begin()
 *
 *  Avantages :
 *    - aucune valeur hardcodée dans la bibliothèque
 *    - configuration propre, centralisée, modifiable facilement
 *    - architecture modulaire : chaque module Discovery 2026 fournit son propre
 *      CanConfigProvider (Master, Booster, DCC2CAN, Gateway…)
 * ============================================================================
 */
class Dcc2CanConfig : public CanConfigProvider {
public:

    /**
     * Nombre total de bus CAN utilisés par le module DCC2CAN.
     * Ici : un seul bus → le CAN interne ESP32.
     */
    uint8_t busCount() const override { return 1; }

    /**
     * Retourne la configuration du bus CAN demandé.
     *
     * index = 0 → bus interne ESP32 (TWAI)
     *
     * Détails :
     *   - vitesse : 500 kbps (réseau Booster Discovery 2026)
     *   - tx/rx  : GPIO 4 / GPIO 5 (brochage standard ESP32)
     *   - cs/int/sck/mosi/miso : NC → car pas de MCP2515
     *   - quartz/tolerance : inutiles pour le bus interne
     */
    const CanBusConfig& bus(uint8_t index) const override {
        static CanBusConfig cfg = {
            true,          // enabled
            500000,        // vitesse CAN (500 kbps)
            GPIO_NUM_4,    // TX ESP32
            GPIO_NUM_5,    // RX ESP32

            // MCP2515 non utilisé → toutes les pins SPI sont NC
            GPIO_NUM_NC,   // cs_pin
            GPIO_NUM_NC,   // int_pin
            GPIO_NUM_NC,   // sck_pin
            GPIO_NUM_NC,   // mosi_pin
            GPIO_NUM_NC,   // miso_pin

            0,             // quartz (non utilisé)
            0              // tolerance (non utilisé)
        };
        return cfg;
    }
};

/**
 * Objet global utilisé par DCC2CAN_main.cpp :
 *
 *      CanInit::begin(DCC2CAN_CONFIG);
 *
 * Il est défini dans DCC2CAN_CanConfig.cpp.
 */
extern Dcc2CanConfig DCC2CAN_CONFIG;
