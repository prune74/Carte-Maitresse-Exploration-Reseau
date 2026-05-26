/*
DCC2CAN_CanBooster.cpp / .h

🎯 Rôle
Driver CAN du module DCC2CAN, utilisant le contrôleur CAN interne de l’ESP32.
Ce module transmet exclusivement les trames DCC logiques vers le bus CAN Booster :
bit DCC + phase (incluant les phases de cutout détectées par le décodeur DCC).

📌 Fonctionnement
- Initialise le contrôleur CAN interne (ACAN_ESP32) avec la vitesse définie
  dans DCCB_CAN_BITRATE.
- Envoie les trames DCC logiques vers les boosters :
    • DCC_BIT → bit DCC + phase (0, 1, cutout)
- Utilise des trames CAN standard (11 bits) pour maximiser la compatibilité.
- Fonctionne indépendamment du CAN Discovery (MCP2515), les deux bus étant
  totalement séparés.

📌 Particularités
- Le bus CAN Booster est unidirectionnel pour DCC2CAN : émission uniquement.
- Le module ne gère ni télémétrie, ni RailCom, ni réception CAN.
- Optimisé pour fonctionner en temps réel avec les tâches FreeRTOS du module
  DCC2CAN (taskDcc, taskCan).
- Le module transmet fidèlement le DCC logique provenant de LaBox vers les
  boosters Discovery 2026.
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
    msg.id  = DCCB_CAN_ID_DCC_BIT;
    msg.len = 2;
    msg.ext = false; // trame standard 11 bits
    msg.data[0] = bit;
    msg.data[1] = phase;

    return ACAN_ESP32::can.tryToSend(msg);
}
