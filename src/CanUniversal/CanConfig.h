#pragma once
#include <stdint.h>
#include <driver/gpio.h>

/**
 * ============================================================================
 *  CanConfig.h
 * ============================================================================
 *  Fichier de configuration centralisé pour la bibliothèque CanUniversal.
 *
 *  Objectifs :
 *    - Aucune valeur en dur dans la bibliothèque
 *    - Configuration simple, claire, modulaire
 *    - Support multi‑bus (CAN[0], CAN[1], CAN[2]…)
 *    - Indépendance totale vis‑à‑vis des rôles (System, Booster, Gateway…)
 *
 *  L’application décide :
 *    - combien de bus CAN sont utilisés
 *    - quels drivers (ESP32 interne, MCP2515)
 *    - quelles vitesses
 *    - quelles broches
 *    - quels rôles (System, Booster, RailCom…)
 *
 *  La bibliothèque ne fait qu’une chose :
 *    → initialiser les bus selon cette configuration.
 * ============================================================================
 */


// -----------------------------------------------------------------------------
// Nombre total de bus CAN utilisés
// -----------------------------------------------------------------------------
// CAN[0] = ESP32 interne
// CAN[1] = MCP2515 externe
//
// Pour ajouter un 3e bus : CAN_COUNT = 3
// -----------------------------------------------------------------------------
static const uint8_t CAN_COUNT = 2;


// ============================================================================
// CAN0 : ESP32 interne (TWAI)
// ============================================================================
static const uint32_t CAN0_SPEED  = 500000;  // 125 ou 250 ou 500 ou 1000 kbit/s

// Broches ESP32 pour le CAN interne (gpio_num_t)
static const gpio_num_t CAN0_TX_PIN = GPIO_NUM_4;
static const gpio_num_t CAN0_RX_PIN = GPIO_NUM_5;


// ============================================================================
// CAN1 : MCP2515 externe (SPI)
// ============================================================================

// Vitesse CAN
static const uint32_t CAN1_SPEED = 250000;  // 125 ou 250 ou 500 kbit/s (robuste, stable)

// Broches MCP2515 (gpio_num_t)
/*
|GPIO   | Peut être CS ?    | Peut être INT ?  | Commentaire    |
| ---   | ---               | ---              | ---            |
| 13    | ✔                | ✔                | Très stable    |
| 14    | ✔                | ✔                | OK             |
| 16    | ✔                | ✔                | Très bon choix |
| 17    | ✔                | ✔                | Très bon choix |
| 21    | ✔                | ✔                | Parfait        |
| 22    | ✔                | ✔                | Parfait        |
| 25    | ✔                | ✔                | OK             |
| 26    | ✔                | ✔                | Très bon choix |
| 27    | ✔                | ✔                | Très bon choix |
| 32    | ✔                | ✔                | Excellent      |
| 33    | ✔                | ✔                | Excellent      |
*/
static const gpio_num_t CAN1_CS_PIN  = GPIO_NUM_13;  // Chip Select
static const gpio_num_t CAN1_INT_PIN = GPIO_NUM_14;  // Interruption

// SPI matériel (imposé par l’ESP32 → VSPI)
static const gpio_num_t MCP2515_SCK  = GPIO_NUM_18;
static const gpio_num_t MCP2515_MOSI = GPIO_NUM_23;
static const gpio_num_t MCP2515_MISO = GPIO_NUM_19;

// Quartz MCP2515
static const uint32_t CAN1_QUARTZ = 8UL * 1000UL * 1000UL;  // 8 ou 16 MHz (Recommendé: 16 Mhz pour les vitesses > 250 kbits/s)

// Tolérance ACAN2515 (en %)
static const uint8_t CAN1_TOLERANCE = 50;


// ============================================================================
// Options avancées (facultatives)
// ============================================================================

// Activer/désactiver un bus (si besoin)
static const bool CAN0_ENABLED = true;
static const bool CAN1_ENABLED = true;  // désactivé par défaut (MCP2515 externe optionnel)

// Possibilité d’ajouter un futur CAN2, CAN3…
// Exemple :
// static const uint32_t CAN2_SPEED = 250000;
// static const gpio_num_t  CAN2_CS_PIN = GPIO_NUM_15;
// static const gpio_num_t  CAN2_INT_PIN = GPIO_NUM_2;
// static const uint32_t CAN2_QUARTZ = 8000000;
// static const uint8_t  CAN2_TOLERANCE = 50;
