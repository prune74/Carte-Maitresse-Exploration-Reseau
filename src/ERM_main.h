#pragma once

#include <Arduino.h>

// ---------------------------------------------------------------------------
// Fonctions principales du module ERM
// ---------------------------------------------------------------------------

// Initialisation complète du système
void ERM_setup();

// Boucle principale FreeRTOS
void ERM_loop();
