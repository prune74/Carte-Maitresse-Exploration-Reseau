#pragma once
#include <Arduino.h>

/* ============================================================================
 *  Debug.h
 *
 *  🎯 Rôle
 *  Fournit un système de journalisation configurable pour l’ensemble du projet.
 *  Les messages sont filtrés selon un niveau global de debug, modifiable à la
 *  volée via la console série.
 *
 *  Niveaux disponibles :
 *      0 → DEBUG_NONE    (aucun log)
 *      1 → DEBUG_ERROR   (erreurs critiques)
 *      2 → DEBUG_WARN    (warnings)
 *      3 → DEBUG_INFO    (informations générales)
 *      4 → DEBUG_VERBOSE (debug détaillé, très verbeux)
 *
 *  ⚠️ En mode réel, DEBUG_VERBOSE est automatiquement bloqué pour éviter toute
 *     perturbation des sections temps réel (DCC, CAN Booster, ISR).
 *
 *  Une macro spéciale LOG_CRITICAL_DCC est fournie pour les zones ultra
 *  sensibles : elle ne logue JAMAIS en mode réel, même si DEBUG_VERBOSE est
 *  activé par erreur.
 * ========================================================================== */

/* ---------------------------------------------------------------------------
 * 🎚️ Niveaux de debug
 * ------------------------------------------------------------------------- */
enum DebugLevel {
    DEBUG_NONE = 0,
    DEBUG_ERROR,
    DEBUG_WARN,
    DEBUG_INFO,
    DEBUG_VERBOSE
};

// Niveau global (défini dans Debug.cpp)
extern DebugLevel DEBUG_LEVEL;

// Indique si le système tourne en mode test (FakeDCC)
extern bool g_isTestMode;

/* ---------------------------------------------------------------------------
 * 📝 Macros de log standard
 * ------------------------------------------------------------------------- */
#define LOG_ERROR(fmt, ...)   do { if (DEBUG_LEVEL >= DEBUG_ERROR)   Serial.printf("[ERROR] " fmt "\n", ##__VA_ARGS__); } while(0)
#define LOG_WARN(fmt, ...)    do { if (DEBUG_LEVEL >= DEBUG_WARN)    Serial.printf("[WARN ] " fmt "\n", ##__VA_ARGS__); } while(0)
#define LOG_INFO(fmt, ...)    do { if (DEBUG_LEVEL >= DEBUG_INFO)    Serial.printf("[INFO ] " fmt "\n", ##__VA_ARGS__); } while(0)
#define LOG_VERBOSE(fmt, ...) do { if (DEBUG_LEVEL >= DEBUG_VERBOSE) Serial.printf("[VERB ] " fmt "\n", ##__VA_ARGS__); } while(0)

/* ---------------------------------------------------------------------------
 * 🔥 Macro spéciale : logs critiques DCC
 *
 *  - Active uniquement en mode test
 *  - Active uniquement si DEBUG_VERBOSE est sélectionné
 *  - Totalement désactivée en mode réel (sécurité temps réel)
 *
 *  Idéal pour :
 *      • logs dans taskDcc
 *      • logs dans CanBooster_sendDccBit
 *      • logs dans le décodeur DCC
 * ------------------------------------------------------------------------- */
#define LOG_CRITICAL_DCC(fmt, ...) \
    do { if (g_isTestMode && DEBUG_LEVEL >= DEBUG_VERBOSE) Serial.printf("[DCC ] " fmt "\n", ##__VA_ARGS__); } while(0)

/* ---------------------------------------------------------------------------
 * 🎮 Commande série pour changer le niveau de debug
 * ------------------------------------------------------------------------- */
void Debug_handleSerialCommand();
