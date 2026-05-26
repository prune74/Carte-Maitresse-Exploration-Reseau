/*
DCC2CAN_Config.h

🎯 Rôle
Fichier de configuration compile‑time pour les modules Discovery 2026 utilisant
le bus CAN Booster. Il regroupe les constantes normalisées nécessaires au
décodage DCC, à la génération des trames CAN et à la supervision du signal.

📌 Contenu
- Identifiants CAN Discovery 2026 :
    • DCC_BIT → bit DCC + phase

- Timings DCC validés :
    • durées des bits 0 et 1
    • seuil de détection du cutout

- Paramètres CAN :
    • vitesse CAN (500 kbps)

- Failsafe :
    • délai de perte du signal DCC
    • temps de réarmement

- Options debug :
    • activation des statistiques internes du décodeur

- Buffers :
    • taille de la queue d'événements DCC

📌 Particularités
- Ce fichier est partagé par DCC2CAN et les modules Booster Discovery 2026.
- Les timings DCC sont conformes aux spécifications NMRA.
*/

#pragma once

// ---------------------------------------------------------------------------
// CAN IDs Discovery 2026
// ---------------------------------------------------------------------------
#define DCCB_CAN_ID_DCC_BIT     0x100   // bit DCC (0/1 + phase)

// ---------------------------------------------------------------------------
// Timings DCC validés Discovery 2026
// ---------------------------------------------------------------------------
#define DCCB_TIMING_BIT1_MIN_US 40
#define DCCB_TIMING_BIT1_MAX_US 80
#define DCCB_TIMING_BIT0_MIN_US 90
#define DCCB_TIMING_BIT0_MAX_US 150

// Détection cutout
#define DCCB_TIMING_CUTOUT_START_US 300 // > → début cutout

// ---------------------------------------------------------------------------
// CAN Booster Discovery 2026
// ---------------------------------------------------------------------------
#define DCCB_CAN_BITRATE 500000 // 500 kbps standard

// ---------------------------------------------------------------------------
// Failsafe Discovery 2026
// ---------------------------------------------------------------------------
#define DCCB_FAILSAFE_TIMEOUT_MS   500   // perte DCC → OFF
#define DCCB_FAILSAFE_COOLDOWN_MS 1000   // réarmement

// ---------------------------------------------------------------------------
// Options debug recommandées
// ---------------------------------------------------------------------------
#define DCCB_MEASURE_STATS 1

// ---------------------------------------------------------------------------
// Taille de la queue d'événements DCC
// ---------------------------------------------------------------------------
#define DCC_EVENT_QUEUE_SIZE 64
