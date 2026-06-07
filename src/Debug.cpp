#include "Debug.h"

// ---------------------------------------------------------------------------
// 🎚️ Niveau de debug par défaut
// ---------------------------------------------------------------------------
// DEBUG_NONE    → silence total
// DEBUG_ERROR   → seulement les erreurs
// DEBUG_WARN    → erreurs + warnings
// DEBUG_INFO    → infos générales (recommandé)
// DEBUG_VERBOSE → debug ultra détaillé

DebugLevel DEBUG_LEVEL = DEBUG_INFO;
