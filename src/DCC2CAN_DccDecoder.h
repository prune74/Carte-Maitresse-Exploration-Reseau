/*
 * DCC2CAN_DccDecoder.h
 *
 * Interface publique du module de décodage DCC.
 *
 * Ce module analyse le signal DCC en temps réel via une ISR et génère
 * des événements (bits, cutout start/end) envoyés dans une queue FreeRTOS.
 *
 * Le traitement lourd est effectué dans les tâches applicatives, tandis que
 * l’ISR se limite à mesurer les timings et à pousser des événements.
 *
 * Les variables internes du décodeur (timings, états, statistiques) sont
 * centralisées dans Variables.h/.cpp afin d’assurer une architecture
 * claire et modulaire.
 */

#pragma once
#include <Arduino.h>
#include "Pins.h"
#include "DCC2CAN_Config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>

/* ---------------------------------------------------------------------------
   TYPES D’ÉVÉNEMENTS DCC
   ---------------------------------------------------------------------------
   Le décodeur produit trois types d’événements :
     - BIT            : bit logique 0 ou 1
     - CUTOUT_START   : début d’un cutout
     - CUTOUT_END     : fin d’un cutout
--------------------------------------------------------------------------- */
enum DccEventType : uint8_t
{
    DCC_EVT_BIT,
    DCC_EVT_CUTOUT_START,
    DCC_EVT_CUTOUT_END
};

/* ---------------------------------------------------------------------------
   STRUCTURE D’UN ÉVÉNEMENT DCC
   ---------------------------------------------------------------------------
   Chaque événement contient :
     - type   : nature de l’événement
     - bit    : valeur logique (0/1) si type = BIT
     - phase  : alternance du signal (0/1)
     - dt_us  : durée mesurée entre deux fronts (microsecondes)
--------------------------------------------------------------------------- */
struct DccEvent
{
    DccEventType type;
    uint8_t bit;
    uint8_t phase;
    uint16_t dt_us;
};

/* ---------------------------------------------------------------------------
   API DU MODULE DÉCODEUR
--------------------------------------------------------------------------- */

// Initialise le décodeur DCC et configure l’ISR
void DccDecoder_begin();

// Lit un événement DCC depuis la queue (non bloquant)
bool DccDecoder_getEvent(DccEvent &ev);

// Retourne les statistiques internes du décodeur
void DccDecoder_getStats(uint32_t &bit0Count, uint32_t &bit1Count,
                         uint32_t &cutoutCount, uint32_t &badTiming);

// Accès direct à la queue d’événements (utilisé par FakeDCC)
QueueHandle_t DccDecoder_getQueue();
