#pragma once
#include "CanConfig.h"

/**
 * ============================================================================
 *  ERM_Config
 * ============================================================================
 *  Fournit la configuration CAN pour la Carte Maîtresse.
 *
 *  La carte Master possède DEUX bus CAN :
 *
 *      - CAN0 : bus interne ESP32 (TWAI) → utilisé pour le Booster
 *      - CAN1 : bus externe MCP2515 → utilisé pour le CanService de Exploration du Réseau maître
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
class ERM_Config : public CanConfigProvider
{
public:
    /**
     * La carte Master utilise 2 bus CAN :
     *   - CAN0 : ESP32 interne
     *   - CAN1 : MCP2515 externe
     */
    uint8_t busCount() const override { return 2; }

    const CanBusConfig &bus(uint8_t index) const override
    {

        extern bool g_isTestMode; // Indication du mode test (FakeDCC) pour activer le loopback sur CAN0

        static CanBusConfig cfg[2] = {

            // -----------------------------------------------------------------
            // CAN0 : ESP32 interne (TWAI) → Booster
            // -----------------------------------------------------------------
            {
                true,        // enabled
                500000,      // speed
                GPIO_NUM_21, // tx_pin
                GPIO_NUM_22, // rx_pin

                GPIO_NUM_NC, // cs_pin
                GPIO_NUM_NC, // int_pin
                GPIO_NUM_NC, // sck_pin
                GPIO_NUM_NC, // mosi_pin
                GPIO_NUM_NC, // miso_pin

                0, // quartz
                0, // tolerance

                g_isTestMode // loopback
            },

            // -----------------------------------------------------------------
            // CAN1 : MCP2515 externe → CanService Canton Controller
            // -----------------------------------------------------------------
            {
                true,        // enabled
                250000,      // speed
                GPIO_NUM_NC, // tx_pin
                GPIO_NUM_NC, // rx_pin

                GPIO_NUM_13, // cs_pin
                GPIO_NUM_14, // int_pin
                GPIO_NUM_18, // sck_pin
                GPIO_NUM_23, // mosi_pin
                GPIO_NUM_19, // miso_pin

                8000000, // quartz
                50,      // tolerance

                g_isTestMode // loopback
            }};

        return cfg[index];
    }
};

/**
 * Objet global utilisé par l’application pour initialiser le CAN :
 *
 *      CanInit::begin(ERM_CAN_CONFIG);
 *
 * Il doit être défini dans ERM_Config.cpp.
 */
extern ERM_Config ERM_CAN_CONFIG;
