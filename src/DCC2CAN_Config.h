/*
 * DCC2CAN_Config.h
 *
 * Fichier de configuration compile‑time pour le module DCC2CAN.
 * Il regroupe l’ensemble des constantes nécessaires au décodage DCC,
 * à la génération des trames CAN et à la supervision du signal.
 *
 * Ce fichier est partagé entre :
 *   - le module DCC2CAN (décodage DCC → CAN)
 *   - les modules Booster compatibles (CAN → signal DCC analogique)
 *
 * Les timings et paramètres définis ici respectent les spécifications NMRA.
 */

#pragma once

/* ---------------------------------------------------------------------------
   IDENTIFIANTS CAN
   ---------------------------------------------------------------------------
   Les trames envoyées par le module DCC2CAN utilisent des identifiants
   normalisés afin d’assurer une compatibilité parfaite avec les Boosters.
--------------------------------------------------------------------------- */
#define DCCB_CAN_ID_DCC_BIT     0x100   // Trame "bit DCC" (bit logique + phase)

/* ---------------------------------------------------------------------------
   TIMINGS DCC (NMRA)
   ---------------------------------------------------------------------------
   Ces valeurs définissent les durées minimales et maximales des bits DCC.
   Elles sont utilisées par le décodeur pour distinguer les bits 0 et 1,
   ainsi que pour détecter les anomalies de signal.
--------------------------------------------------------------------------- */
#define DCCB_TIMING_BIT1_MIN_US 40
#define DCCB_TIMING_BIT1_MAX_US 80
#define DCCB_TIMING_BIT0_MIN_US 90
#define DCCB_TIMING_BIT0_MAX_US 150

// Détection du cutout (absence volontaire de signal)
#define DCCB_TIMING_CUTOUT_START_US 300   // Durée > 300 µs → début du cutout

/* ---------------------------------------------------------------------------
   PARAMÈTRES CAN
   ---------------------------------------------------------------------------
   Le bus CAN utilisé pour transporter les bits DCC fonctionne en 500 kbps.
--------------------------------------------------------------------------- */
#define DCCB_CAN_BITRATE 500000   // 500 kbps

/* ---------------------------------------------------------------------------
   FAILSAFE DCC
   ---------------------------------------------------------------------------
   Le failsafe protège le réseau en cas de perte du signal DCC :
     - timeout : durée maximale sans bit reçu
     - cooldown : délai avant réactivation après une erreur
--------------------------------------------------------------------------- */
#define DCCB_FAILSAFE_TIMEOUT_MS   500    // Perte DCC → OFF
#define DCCB_FAILSAFE_COOLDOWN_MS 1000    // Réarmement

/* ---------------------------------------------------------------------------
   OPTIONS DE MESURE ET DE DIAGNOSTIC
   ---------------------------------------------------------------------------
   Active les statistiques internes du décodeur (compteurs, erreurs…).
--------------------------------------------------------------------------- */
#define DCCB_MEASURE_STATS 1

/* ---------------------------------------------------------------------------
   TAILLE DE LA QUEUE D’ÉVÉNEMENTS DCC
   ---------------------------------------------------------------------------
   La queue transporte les paquets DCC décodés vers la tâche de traitement.
--------------------------------------------------------------------------- */
#define DCC_EVENT_QUEUE_SIZE 64
