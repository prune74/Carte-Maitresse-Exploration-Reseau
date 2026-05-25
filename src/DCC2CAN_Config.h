/*
DCC2CAN_Config.h

🎯 Rôle
Fichier de configuration compile‑time du module DCC_CAN-Booster.
Il regroupe toutes les constantes normalisées Discovery 2026 utilisées par
le Booster : IDs CAN, timings DCC, paramètres failsafe, options debug et
périodes de télémétrie.

Ce fichier constitue la référence officielle pour tous les modules Booster.

📌 Contenu
- Identifiants CAN standardisés Discovery 2026 :
    • DCC_BIT      → bit DCC + phase
    • CUTOUT       → début/fin de cutout
    • TELEMETRY    → courant, tension, état
    • RAILCOM      → adresse détectée

- Timings DCC validés :
    • durées des bits 0 et 1
    • seuils de détection du cutout
    • marges de sécurité

- Paramètres CAN Booster :
    • vitesse CAN (500 kbps)
    • format standard 11 bits

- Failsafe :
    • délai de perte du signal DCC
    • temps de réarmement

- Télémétrie :
    • période d’envoi des mesures

- Options debug :
    • niveau de verbosité série
    • statistiques internes
    • mode oscilloscope DCC

📌 Particularités
- Ce fichier est partagé par tous les modules Booster pour garantir une
  compatibilité totale entre les firmwares Discovery 2026.
- Les valeurs sont choisies pour assurer :
    • une détection fiable du signal DCC
    • une charge CAN minimale
    • une supervision robuste
- Les timings DCC sont conformes aux spécifications NMRA.

🔗 Dépendances
Aucune : ce fichier est autonome et inclus par tous les modules du Booster.
*/

#pragma once

// ============================================================================
//  Discovery 2026 - Profil standardisé pour modules DCC → CAN Booster
//  Ce fichier définit :
//   - IDs CAN officiels
//   - timings DCC validés
//   - comportement failsafe
//   - options debug recommandées
//   - télémétrie standard
// ============================================================================

// ---------------------------------------------------------------------------
// CAN IDs Discovery 2026
// ---------------------------------------------------------------------------
#define DCCB_CAN_ID_DCC_BIT 0x100   // bit DCC (0/1 + phase)
#define DCCB_CAN_ID_CUTOUT 0x101    // cutout local/global
#define DCCB_CAN_ID_TELEMETRY 0x102 // télémétrie booster
#define DCCB_CAN_ID_RAILCOM 0x103   // RailCom (EXSA)

// ---------------------------------------------------------------------------
// Timings DCC validés Discovery 2026
// ---------------------------------------------------------------------------
#define DCCB_TIMING_BIT1_MIN_US 40
#define DCCB_TIMING_BIT1_MAX_US 80
#define DCCB_TIMING_BIT0_MIN_US 90
#define DCCB_TIMING_BIT0_MAX_US 150

// Détection cutout
#define DCCB_TIMING_CUTOUT_START_US 300 // > → début cutout
#define DCCB_TIMING_CUTOUT_END_GAP 80   // marge pour fin cutout

// ---------------------------------------------------------------------------
// CAN Booster Discovery 2026
// ---------------------------------------------------------------------------
#define DCCB_CAN_BITRATE 500000 // 500 kbps standard

// ---------------------------------------------------------------------------
// Failsafe Discovery 2026
// ---------------------------------------------------------------------------
#define DCCB_FAILSAFE_TIMEOUT_MS 500   // perte DCC → OFF
#define DCCB_FAILSAFE_COOLDOWN_MS 1000 // réarmement

// ---------------------------------------------------------------------------
// Télémétrie Discovery 2026
// ---------------------------------------------------------------------------
#define DCCB_TELEMETRY_PERIOD_MS 1000 // envoi toutes les 1s

// ---------------------------------------------------------------------------
// Options debug recommandées
// ---------------------------------------------------------------------------
#define DCCB_DEBUG_SERIAL 1 // 0=off, 1=normal, 2=verbeux
#define DCCB_MEASURE_STATS 1
#define DCCB_SCOPE_MODE 0 // 1 = oscilloscope DCC

// ---------------------------------------------------------------------------
// Taille de la queue d'événements DCC
// ---------------------------------------------------------------------------
#define DCC_EVENT_QUEUE_SIZE 64
