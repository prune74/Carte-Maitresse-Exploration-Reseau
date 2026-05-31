/*
DiscoveryMaster_Config.h

🎯 Rôle
Configuration locale du DiscoveryMaster.
Ne contient PAS le protocole Discovery 2026.
Toutes les commandes CAN / PROTO_xx sont dans Discovery_Protocol.h.
*/

#pragma once
#include <Arduino.h>
#include <ACAN2515.h>

/* ----- Informations projet ----- */
#define PROJECT "Discovery Master Board"
#define VERSION "v0.1"

/* ----- Debug ----- */
#define DEBUG
#define debug Serial

/* ----- Constantes système ----- */
#define NO_ID 255
#define NO_PIN 255
#define NB_SAT 30

/* ----- WiFi : Mode AP (optionnel) ----- */
//#define WIFI_AP_MODE

/* ============================================================
 *  🟥 DISCOVERY CAN 11 bits — Messages globaux Master ↔ SA
 * ============================================================
 *
 *  Ces IDs ne suivent PAS le format 29 bits Märklin.
 *  Ils sont utilisés pour la supervision globale.
 */

/* Heartbeat SA → Master */
#define DISCOVERY_CAN_ID_HEARTBEAT          0x200  // DLC=2 : [ID_H][ID_L]

/* STOP global Master → SA (puis SA → EXSA via RS485) */
#define DISCOVERY_CAN_ID_EMERGENCY_STOP     0x201  // DLC=0 : STOP global

/* CLEAR STOP global Master → SA */
#define DISCOVERY_CAN_ID_CLEAR_STOP         0x202  // DLC=0 : levée STOP global
