#include "Debug.h"

/*
 * Debug.cpp
 *
 * 🎯 Rôle
 * Implémentation du système de debug global pour le projet.
 *
 * Ce module définit :
 *   • le niveau de debug global (DEBUG_LEVEL)
 *   • les macros de log conditionnelles (LOG_ERROR, LOG_WARN, etc.)
 *   • une commande série pour changer le niveau de debug à la volée
 *
 * Il est utilisé dans tous les modules du projet pour assurer une
 * journalisation cohérente et configurable.
 *
 * Le niveau de debug peut être ajusté en envoyant un caractère via
 * la console série :
 *   '0' → DEBUG_NONE (aucun log)
 *   '1' → DEBUG_ERROR (erreurs seulement)
 *   '2' → DEBUG_WARN (erreurs + warnings)
 *   '3' → DEBUG_INFO (infos générales)
 *   '4' → DEBUG_VERBOSE (debug détaillé)
 *
 * Par défaut, le niveau est réglé sur DEBUG_INFO pour un bon équilibre
 * entre information et lisibilité.
 *
 * Les macros de log utilisent Serial.printf pour formater les messages,
 * et sont conditionnées par le niveau de debug global.
 *
 * Exemple d’utilisation dans un module :
 *
 *   LOG_INFO("Valeur = %d", x);
 *
 * Cela affichera "Valeur = 42" si DEBUG_LEVEL >= DEBUG_INFO, ou rien sinon.
 */

// ---------------------------------------------------------------------------
// 🎚️ Niveau de debug par défaut
// ---------------------------------------------------------------------------
// DEBUG_NONE    → silence total
// DEBUG_ERROR   → seulement les erreurs
// DEBUG_WARN    → erreurs + warnings
// DEBUG_INFO    → infos générales (recommandé)
// DEBUG_VERBOSE → debug ultra détaillé

DebugLevel DEBUG_LEVEL = DEBUG_INFO;

// ---------------------------------------------------------------------------
// 🎮 Commande série pour changer le niveau de debug
// ---------------------------------------------------------------------------
void Debug_handleSerialCommand()
{
    if (!Serial.available())
        return;

    char c = Serial.read();

    switch (c)
    {
    case '0':
        DEBUG_LEVEL = DEBUG_NONE;
        Serial.println("[DEBUG] Niveau = 0 (NONE)");
        break;

    case '1':
        DEBUG_LEVEL = DEBUG_ERROR;
        Serial.println("[DEBUG] Niveau = 1 (ERROR)");
        break;

    case '2':
        DEBUG_LEVEL = DEBUG_WARN;
        Serial.println("[DEBUG] Niveau = 2 (WARN)");
        break;

    case '3':
        DEBUG_LEVEL = DEBUG_INFO;
        Serial.println("[DEBUG] Niveau = 3 (INFO)");
        break;

    case '4':
        DEBUG_LEVEL = DEBUG_VERBOSE;
        Serial.println("[DEBUG] Niveau = 4 (VERBOSE)");
        break;

    default:
        Serial.println("[DEBUG] Commande inconnue. Utilise 0-4.");
        break;
    }
}
