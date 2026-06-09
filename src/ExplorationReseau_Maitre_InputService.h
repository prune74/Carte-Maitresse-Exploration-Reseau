#pragma once
#include <Arduino.h>
#include "CanMsg.h"

// ---------------------------------------------------------------------------
// Classe ERM_InputService
// Gestion des entrées physiques de la Carte Maîtresse.
// ---------------------------------------------------------------------------
class ERM_InputService
{
public:
    // Initialisation du module
    void begin();

    // Boucle de supervision du bouton CLEAR STOP
    void loop();

private:
    // Mémorise l’état précédent du bouton
    bool _prevBtn = true;
};
