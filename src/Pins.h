#pragma once
#include <Arduino.h>

/* ============================================================
   🟥 LED STOP — Carte Maîtresse Exploration Réseau
   ------------------------------------------------------------
   • LED rouge
   • Allumée lorsque le STOP global (CAN 0x201) est actif
   • Permet un diagnostic immédiat sans interface Web
   ============================================================ */
static const gpio_num_t PIN_LED_STOP = GPIO_NUM_25;

/* ============================================================
   🟦 Bouton CLEAR STOP — Carte Maîtresse Exploration Réseau
   ------------------------------------------------------------
   • Bouton physique
   • Appui court → envoie CLEAR STOP (CAN 0x202)
   • Pull-up interne recommandé
   • Permet de lever STOP même sans interface Web
   ============================================================ */
static const gpio_num_t PIN_BTN_CLEAR_STOP = GPIO_NUM_26;

/* ============================================================
   🟧 LED CC OFFLINE — Carte Maîtresse Exploration Réseau
   ------------------------------------------------------------
   • LED orange (ou autre couleur)
   • Allumée lorsqu’un CC du réseau devient OFFLINE
   • Indique une anomalie topologique détectée par ERS
   ============================================================ */
static const gpio_num_t PIN_LED_CC_OFFLINE = GPIO_NUM_27;

/*
Pins pour DCC2CAN:

🎯 Rôle
Définition centralisée des broches utilisées par le module DCC2CAN.
Ce fichier regroupe toutes les constantes liées au câblage matériel du
convertisseur DCC → CAN Booster, afin de garantir une configuration claire,
cohérente et facilement modifiable.

📌 Contenu
- PIN_DCC_IN  : entrée logique du signal DCC (issue du XOR SN74LVC1G86)
- PIN_CAN_TX  : voir fichier CAN_Config.h
- PIN_CAN_RX  : voir fichier CAN_Config.h
- PIN_LED     : LED de debug (clignote à chaque événement DCC)

📌 Particularités
- Les broches sont définies sous forme de constantes gpio_num_t pour une
  compatibilité totale avec les API ESP-IDF et Arduino.
- Ce fichier est utilisé par les modules DccDecoder, CanBooster, tâches
  FreeRTOS et CLI du firmware DCC2CAN.
- Les broches correspondent à la carte ESP32 utilisée pour le module DCC2CAN.
*/

// Entrée DCC logique (sortie XOR SN74LVC1G86)
static const gpio_num_t PIN_DCC_IN = GPIO_NUM_27;

// LED debug
static const gpio_num_t PIN_LED = GPIO_NUM_2;
