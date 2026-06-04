/*
DCC2CAN_Pins.h

🎯 Rôle
Définition centralisée des broches utilisées par le module DCC2CAN.
Ce fichier regroupe toutes les constantes liées au câblage matériel du
convertisseur DCC → CAN Booster, afin de garantir une configuration claire,
cohérente et facilement modifiable.

📌 Contenu
- PIN_DCC_IN  : entrée logique du signal DCC (issue du XOR SN74LVC1G86)
- PIN_CAN_TX  : broche TX du contrôleur CAN interne de l’ESP32
- PIN_CAN_RX  : broche RX du contrôleur CAN interne de l’ESP32
- PIN_LED     : LED de debug (clignote à chaque événement DCC)

📌 Particularités
- Les broches sont définies sous forme de constantes gpio_num_t pour une
  compatibilité totale avec les API ESP-IDF et Arduino.
- Ce fichier est utilisé par les modules DccDecoder, CanBooster, tâches
  FreeRTOS et CLI du firmware DCC2CAN.
- Les broches correspondent à la carte ESP32 utilisée pour le module DCC2CAN.
*/


#pragma once
#include <Arduino.h>

// Entrée DCC logique (sortie XOR SN74LVC1G86)
static const gpio_num_t PIN_DCC_IN = GPIO_NUM_27;

// LED debug
static const gpio_num_t PIN_LED = GPIO_NUM_2;
