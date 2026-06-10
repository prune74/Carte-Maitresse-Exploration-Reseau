/*
 * DCC2CAN_State.h
 *
 * Interface publique du module gérant l’état logique du flux DCC.
 *
 * Ce composant centralise :
 *   - le dernier événement DCC reçu (bit, phase, timing, type)
 *   - l’état de supervision du signal (RUNNING, LOST, RECOVERY)
 *   - le timestamp du dernier événement
 *
 * Le module fournit trois fonctions essentielles :
 *   - mise à jour de l’état depuis un événement DCC
 *   - envoi du bit courant vers le Booster via CAN
 *   - supervision du signal DCC (failsafe)
 *
 * Toutes les variables globales associées à l’état (g_state, mutex, timers)
 * sont définies dans Variables.h/.cpp afin d’assurer une architecture
 * claire, modulaire et testable.
 */

#pragma once
#include <Arduino.h>
#include "DCC2CAN_DccDecoder.h"
#include "DCC2CAN_CanBooster.h"
#include "DCC2CAN_Config.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/* ---------------------------------------------------------------------------
   ÉTATS DE SUPERVISION DU SIGNAL DCC
   ---------------------------------------------------------------------------
   Le système peut se trouver dans trois états :
     - RUNNING  : signal DCC valide
     - DCC_LOST : perte du signal (timeout)
     - RECOVERY : période de réarmement après une perte
--------------------------------------------------------------------------- */
enum BoosterSupervisionState : uint8_t
{
    BSTATE_RUNNING = 0,
    BSTATE_DCC_LOST = 1,
    BSTATE_RECOVERY = 2
};

/* ---------------------------------------------------------------------------
   STRUCTURE DE L’ÉTAT LOGIQUE DU BOOSTER
   ---------------------------------------------------------------------------
   lastEvent      : dernier événement DCC reçu
   status         : état de supervision
   lastEventTime  : timestamp du dernier événement (millis)
--------------------------------------------------------------------------- */
struct BoosterRuntimeState
{
    DccEvent lastEvent;
    BoosterSupervisionState status;
    uint32_t lastEventTime;
};

/* ---------------------------------------------------------------------------
   VARIABLES GLOBALES (définies dans DCC2CAN_Variables.cpp)
--------------------------------------------------------------------------- */
extern volatile BoosterRuntimeState g_state;
extern SemaphoreHandle_t gStateUpdateMutex;

/* ---------------------------------------------------------------------------
   API DU MODULE ÉTAT
--------------------------------------------------------------------------- */

// Initialise le mutex et prépare l’état interne
void BoosterState_init();

// Met à jour l’état depuis un événement DCC (appelé par taskDcc)
void BoosterState_updateFromDcc(const volatile DccEvent &ev);

// Envoie le bit courant sur le bus CAN (appelé par taskCan)
void BoosterState_sendCan();

// Supervise la présence du signal DCC (appelé par taskSupervision)
void BoosterState_supervise();
