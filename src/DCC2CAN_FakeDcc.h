/*
 * DCC2CAN_FakeDcc.h
 *
 * Interface publique du module de simulation DCC.
 *
 * Ce module génère artificiellement des événements DCC afin de permettre
 * le test complet du pipeline DCC2CAN sans matériel réel. Il remplace
 * entièrement l’ISR du décodeur lorsque le système fonctionne en mode test.
 *
 * FakeDCC utilise :
 *   - la même queue FreeRTOS que le décodeur réel
 *   - un tick régulier générant des bits DCC synthétiques
 *
 * La simulation est volontairement simple : elle produit un flux régulier
 * de bits 1, suffisant pour valider le fonctionnement du Booster et du CAN.
 */

#pragma once
#include <Arduino.h>
#include "DCC2CAN_DccDecoder.h"

/* ---------------------------------------------------------------------------
   MODE SIMULATION
   ---------------------------------------------------------------------------
   Lorsque DCC_FAKE_MODE est vrai :
     - l’ISR réelle n’est pas attachée
     - FakeDCC génère les événements DCC
--------------------------------------------------------------------------- */
extern bool DCC_FAKE_MODE;

/* ---------------------------------------------------------------------------
   API DU MODULE FAKE DCC
--------------------------------------------------------------------------- */

// Initialise la simulation (récupère la queue du décodeur)
void FakeDcc_begin();

// Génère un bit DCC synthétique (à appeler régulièrement)
void FakeDcc_tick();
