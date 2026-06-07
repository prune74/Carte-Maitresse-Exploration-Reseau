#pragma once
#include <Arduino.h>

// ---------------------------------------------------------------------------
// 🎚️ Niveaux de debug
// ---------------------------------------------------------------------------
enum DebugLevel {
    DEBUG_NONE = 0,   // Aucun log
    DEBUG_ERROR,      // Erreurs critiques
    DEBUG_WARN,       // Avertissements
    DEBUG_INFO,       // Informations générales
    DEBUG_VERBOSE     // Debug détaillé
};

// Niveau global (défini dans Debug.cpp)
extern DebugLevel DEBUG_LEVEL;

// ---------------------------------------------------------------------------
// 📝 Macros de log
// ---------------------------------------------------------------------------
// Utilisation : LOG_INFO("Valeur = %d", x);

#define LOG_ERROR(fmt, ...)   do { if (DEBUG_LEVEL >= DEBUG_ERROR)   Serial.printf("[ERROR] " fmt "\n", ##__VA_ARGS__); } while(0)
#define LOG_WARN(fmt, ...)    do { if (DEBUG_LEVEL >= DEBUG_WARN)    Serial.printf("[WARN ] " fmt "\n", ##__VA_ARGS__); } while(0)
#define LOG_INFO(fmt, ...)    do { if (DEBUG_LEVEL >= DEBUG_INFO)    Serial.printf("[INFO ] " fmt "\n", ##__VA_ARGS__); } while(0)
#define LOG_VERBOSE(fmt, ...) do { if (DEBUG_LEVEL >= DEBUG_VERBOSE) Serial.printf("[VERB ] " fmt "\n", ##__VA_ARGS__); } while(0)

