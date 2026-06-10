/*
 * DCC2CAN_CanBooster.h
 *
 * Interface publique du driver CAN Booster.
 *
 * Ce module assure l’envoi des trames CAN contenant :
 *   - le bit DCC logique (0 ou 1)
 *   - la phase du signal (0 ou 1)
 *
 * Le bus CAN Booster est unidirectionnel pour DCC2CAN : émission uniquement.
 * Le driver utilise le contrôleur CAN interne de l’ESP32 via ACAN_ESP32.
 */

#pragma once
#include <ACAN_ESP32.h>
#include "DCC2CAN_DccDecoder.h"

/* ---------------------------------------------------------------------------
   INITIALISATION DU DRIVER CAN BOOSTER
   ---------------------------------------------------------------------------
   Configure le contrôleur CAN interne de l’ESP32 :
     - bitrate défini dans DCC2CAN_Config.h
     - pins TX/RX définis dans DCC2CAN_Pins.h
--------------------------------------------------------------------------- */
void CanBooster_begin();

/* ---------------------------------------------------------------------------
   ENVOI D’UN BIT DCC SUR LE BUS CAN BOOSTER
   ---------------------------------------------------------------------------
   bit   : valeur logique (0 ou 1)
   phase : alternance du signal (0 ou 1)
   Retourne true si la trame a été envoyée avec succès.
--------------------------------------------------------------------------- */
bool CanBooster_sendDccBit(uint8_t bit, uint8_t phase);
