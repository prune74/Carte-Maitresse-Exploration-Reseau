#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

/*
 * Variables.h
 *
 * 🎯 Rôle
 * Ce fichier centralise **toutes les variables globales** du firmware ERM.
 *
 * Pourquoi centraliser ?
 *   • éviter les redéfinitions (linker errors)
 *   • garantir une architecture claire et modulaire
 *   • rendre l’état global du système lisible et maintenable
 *   • simplifier l’intégration entre les modules (DCC2CAN, ERM, ERS…)
 *
 * Chaque section est organisée par module :
 *   - DCC2CAN : décodeur DCC + Booster
 *   - ERM     : maître d’exploration réseau
 *   - ERS     : Watchdog de supervision
 *
 * Toutes les variables sont déclarées ici en `extern` et définies dans
 * Variables.cpp.
 */

#include "DCC2CAN_DccDecoder.h"
#include "DCC2CAN_State.h"

#include "ERM_CC_Manager.h"
#include "CanBus.h"

// 🔥 Forward declarations pour casser les boucles d’includes
class ERM_CanService;
class ERM_WebHandler;

/* ---------------------------------------------------------------------------
   🟦 DCC2CAN — VARIABLES DU DÉCODEUR DCC
--------------------------------------------------------------------------- */

// Timestamp du dernier front DCC (micros)
extern volatile uint32_t dcc_lastEdge;

// Phase courante du signal (0/1)
extern volatile uint8_t dcc_currentPhase;

// Indique si le décodeur est dans un cutout
extern volatile bool dcc_inCutout;

// Queue d’événements DCC (ISR → taskDcc)
extern QueueHandle_t dccQueue;

/* ---------------------------------------------------------------------------
   🟦 DCC2CAN — STATISTIQUES DU DÉCODEUR
--------------------------------------------------------------------------- */

extern volatile uint32_t dcc_bit0Count;
extern volatile uint32_t dcc_bit1Count;
extern volatile uint32_t dcc_cutoutCount;
extern volatile uint32_t dcc_badTiming;

/* ---------------------------------------------------------------------------
   🟦 DCC2CAN — ÉTAT LOGIQUE DU BOOSTER
--------------------------------------------------------------------------- */

extern volatile BoosterRuntimeState g_state;

// Mutex protégeant g_state
extern SemaphoreHandle_t gStateUpdateMutex;
extern StaticSemaphore_t gStateUpdateMutexBuffer;

// Timer interne pour le mode RECOVERY
extern uint32_t recoveryStartTime;

/* ---------------------------------------------------------------------------
   🟩 ERM — STOP / SAVE / RESTART
--------------------------------------------------------------------------- */

extern volatile uint8_t g_stopState;
extern volatile uint8_t g_saveState;
extern volatile uint8_t g_restartState;

/* ---------------------------------------------------------------------------
   🟩 ERM — Mode test
--------------------------------------------------------------------------- */

extern bool g_isTestMode;

/* ---------------------------------------------------------------------------
   🟩 ERM — Gestion des Canton Controllers
--------------------------------------------------------------------------- */

extern ERM_CC_Manager CC_Manager;
extern volatile uint8_t g_ccOnlineCount;
extern volatile uint8_t g_lastSatId;

/* ---------------------------------------------------------------------------
   🟩 ERM — CAN : tableau de bus
--------------------------------------------------------------------------- */

extern CanBus *CAN[2];
extern volatile uint32_t g_lastCanError;

/* ---------------------------------------------------------------------------
   🟩 ERM — Logs
--------------------------------------------------------------------------- */

extern String g_lastLogMessage;

/* ---------------------------------------------------------------------------
   🟩 ERM — Services globaux
--------------------------------------------------------------------------- */

extern ERM_CanService canService;
extern ERM_WebHandler webHandler;

/* ---------------------------------------------------------------------------
   🟥 ERS — PARAMÈTRES DU WATCHDOG
--------------------------------------------------------------------------- */

extern uint32_t ERS_TIMEOUT_MS;
extern uint32_t ERS_SUP_PERIOD_MS;
extern uint32_t ERS_RX_PERIOD_MS;

/* ---------------------------------------------------------------------------
   🟥 ERS — TABLE DES HEARTBEATS
--------------------------------------------------------------------------- */

extern uint32_t ers_lastHeartbeat[NB_CC];

extern SemaphoreHandle_t ersHeartbeatMutex;
extern StaticSemaphore_t ersHeartbeatMutexBuffer;

/* ---------------------------------------------------------------------------
   🟥 ERS — COMPTEURS ET STATISTIQUES
--------------------------------------------------------------------------- */

extern volatile uint32_t ers_heartbeatCount;
extern volatile uint32_t ers_timeoutCount;
extern volatile uint32_t ers_recoveryCount;

/* ---------------------------------------------------------------------------
   🟥 ERS — ÉTAT INTERNE DU WATCHDOG
--------------------------------------------------------------------------- */

extern volatile uint8_t ers_onlineCount;
extern volatile uint16_t ers_lastSatId;

extern bool ers_enabled;

/* ---------------------------------------------------------------------------
   🟥 ERS — TIMERS INTERNES
--------------------------------------------------------------------------- */

extern uint32_t ers_lastSupervisionTime;
extern uint32_t ers_lastStateChangeTime;
