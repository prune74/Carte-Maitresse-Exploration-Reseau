#include "Debug.h"

/*
 * Debug.cpp
 *
 * 🎯 Rôle
 * Implémentation du système de debug global.
 *
 * Ce module gère :
 *   • le niveau global de debug (DEBUG_LEVEL)
 *   • la commande série permettant de modifier ce niveau
 *   • la protection automatique contre DEBUG_VERBOSE en mode réel
 */

// Indique si le système tourne en mode test (FakeDCC)
extern bool g_isTestMode;

// Niveau de debug par défaut
DebugLevel DEBUG_LEVEL = DEBUG_INFO;

/* ---------------------------------------------------------------------------
 * 🔒 Fonction interne : applique un niveau de debug sécurisé
 * ------------------------------------------------------------------------- */
static void Debug_setLevel(DebugLevel lvl)
{
    // Interdiction du mode VERBOSE en mode réel
    if (!g_isTestMode && lvl == DEBUG_VERBOSE)
    {
        Serial.println("[DEBUG] VERBOSE interdit en mode réel");
        return;
    }

    DEBUG_LEVEL = lvl;
}

/* ---------------------------------------------------------------------------
 * 🎮 Commande série pour changer le niveau de debug
 * ------------------------------------------------------------------------- */
void Debug_handleSerialCommand()
{
    if (!Serial.available())
        return;

    char c = Serial.read();

    switch (c)
    {
    case '0':
        Debug_setLevel(DEBUG_NONE);
        Serial.println("[DEBUG] Niveau = 0 (NONE)");
        break;

    case '1':
        Debug_setLevel(DEBUG_ERROR);
        Serial.println("[DEBUG] Niveau = 1 (ERROR)");
        break;

    case '2':
        Debug_setLevel(DEBUG_WARN);
        Serial.println("[DEBUG] Niveau = 2 (WARN)");
        break;

    case '3':
        Debug_setLevel(DEBUG_INFO);
        Serial.println("[DEBUG] Niveau = 3 (INFO)");
        break;

    case '4':
        Debug_setLevel(DEBUG_VERBOSE);
        Serial.println("[DEBUG] Niveau = 4 (VERBOSE)");
        break;

    default:
        Serial.println("[DEBUG] Commande inconnue. Utilise 0-4.");
        break;
    }
}
