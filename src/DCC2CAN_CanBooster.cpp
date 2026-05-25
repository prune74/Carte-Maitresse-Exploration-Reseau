/*
DCC2CAN_CanBooster.cpp / .h

🎯 Rôle
Driver CAN du module Booster, utilisant le contrôleur CAN interne de l’ESP32.
Ce module encapsule toutes les fonctions d’envoi de trames liées au DCC :
bits, cutout, télémétrie et RailCom.

📌 Fonctionnement
- Initialise le contrôleur CAN interne (ACAN_ESP32) avec la vitesse définie
  dans DCCB_CAN_BITRATE.
- Envoie les trames DCC encapsulées vers le bus CAN Booster :
    • DCC_BIT        → bit DCC + phase
    • CUTOUT         → début/fin de cutout
    • TELEMETRY      → courant, tension, état du booster
    • RAILCOM        → adresse détectée en RailCom
- Utilise des trames CAN standard (11 bits) pour maximiser la compatibilité.
- Fonctionne indépendamment du CAN Discovery (MCP2515), les deux bus étant
  totalement séparés.

📌 Particularités
- Le module est optimisé pour fonctionner en temps réel avec les tâches FreeRTOS
  du Booster (taskDcc, taskCan, taskCanRx).
- Les valeurs envoyées sont compactées pour réduire la charge CAN :
    • courant en pas de 10 mA
    • tension en pas de 100 mV
- Le failsafe est géré dans taskCan(), mais les trames d’état sont envoyées ici.
*/

#include "DCC2CAN_CanBooster.h"

void CanBooster_begin()
{
    ACAN_ESP32_Settings settings(DCCB_CAN_BITRATE);
    settings.mTxPin = PIN_CAN_TX;
    settings.mRxPin = PIN_CAN_RX;

    uint32_t err = ACAN_ESP32::can.begin(settings);
    if (err != 0)
    {
        Serial.printf("CAN error 0x%X\n", err);
    }
    else
    {
        Serial.println("CAN Booster OK");
    }
}

bool CanBooster_sendDccBit(uint8_t bit, uint8_t phase)
{
    CANMessage msg;
    msg.id = DCCB_CAN_ID_DCC_BIT;
    msg.len = 2;
    msg.ext = false; // trame standard 11 bits
    msg.data[0] = bit;
    msg.data[1] = phase;
    return ACAN_ESP32::can.tryToSend(msg);
}

bool CanBooster_sendCutout(bool local, bool global)
{
    CANMessage msg;
    msg.id = DCCB_CAN_ID_CUTOUT;
    msg.len = 2;
    msg.ext = false;
    msg.data[0] = local ? 1 : 0;
    msg.data[1] = global ? 1 : 0;
    return ACAN_ESP32::can.tryToSend(msg);
}

bool CanBooster_sendTelemetry(uint16_t mA, uint16_t mV, BoosterState st)
{
    CANMessage msg;
    msg.id = DCCB_CAN_ID_TELEMETRY;
    msg.len = 3;
    msg.ext = false;
    msg.data[0] = (uint8_t)(mA / 10);  // x10 mA
    msg.data[1] = (uint8_t)(mV / 100); // x100 mV
    msg.data[2] = (uint8_t)st;
    return ACAN_ESP32::can.tryToSend(msg);
}

bool CanBooster_sendRailcomAddress(uint16_t addr)
{
    CANMessage msg;
    msg.id = DCCB_CAN_ID_RAILCOM;
    msg.len = 2;
    msg.ext = false;
    msg.data[0] = (uint8_t)(addr & 0xFF);
    msg.data[1] = (uint8_t)(addr >> 8);
    return ACAN_ESP32::can.tryToSend(msg);
}
