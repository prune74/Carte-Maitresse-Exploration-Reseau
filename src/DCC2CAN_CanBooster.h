/*
 * DCC2CAN_CanBooster.h
 *
 * Interface publique du driver CAN Booster.
 *
 * Ce module assure désormais l’envoi de deux types de trames CAN :
 *
 *   1️⃣  Trames DCC BIT
 *       - bit logique (0 ou 1)
 *       - phase du signal (0 ou 1)
 *
 *   2️⃣  Trames CUTOUT
 *       - CUTOUT_START  (0)
 *       - CUTOUT_END    (1)
 *
 * L’objectif est de permettre au Booster de reconstruire le signal DCC
 * et de synchroniser parfaitement la fenêtre RailCom sans avoir à
 * deviner ou reconstruire le cutout localement.
 *
 * Le bus CAN Booster est unidirectionnel pour ce module : émission uniquement.
 */

#pragma once
#include <ACAN_ESP32.h>
#include "DCC2CAN_DccDecoder.h"

/* ---------------------------------------------------------------------------
   INITIALISATION DU DRIVER CAN BOOSTER
   ---------------------------------------------------------------------------
   Configure le contrôleur CAN interne de l’ESP32 :
     - bitrate défini dans DCC2CAN_Config.h
     - pins TX/RX définis dans Pins.h
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

/* ---------------------------------------------------------------------------
   ENVOI D’UN ÉVÉNEMENT CUTOUT SUR LE BUS CAN BOOSTER
   ---------------------------------------------------------------------------
   cutoutType :
       0 → CUTOUT_START
       1 → CUTOUT_END
   Retourne true si la trame a été envoyée avec succès.
--------------------------------------------------------------------------- */
bool CanBooster_sendCutout(uint8_t cutoutType);
