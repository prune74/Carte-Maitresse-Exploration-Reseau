#include "Variables.h"

#include "ExplorationReseau_Maitre_CanService.h"
#include "ExplorationReseau_Maitre_WebHandler.h"

/*
 * Variables.cpp
 *
 * 🎯 Rôle
 * Définition **unique** de toutes les variables globales du firmware ERM.
 *
 * Ce fichier est le miroir exact de Variables.h.
 * Chaque variable est définie ici, une seule fois, pour éviter toute
 * redéfinition lors de l’édition de liens.
 */

/* ---------------------------------------------------------------------------
   🟦 DCC2CAN — VARIABLES DU DÉCODEUR DCC
--------------------------------------------------------------------------- */

volatile uint32_t dcc_lastEdge = 0;
volatile uint8_t  dcc_currentPhase = 0;
volatile bool     dcc_inCutout = false;

QueueHandle_t dccQueue = nullptr;

/* ---------------------------------------------------------------------------
   🟦 DCC2CAN — STATISTIQUES DU DÉCODEUR
--------------------------------------------------------------------------- */

volatile uint32_t dcc_bit0Count   = 0;
volatile uint32_t dcc_bit1Count   = 0;
volatile uint32_t dcc_cutoutCount = 0;
volatile uint32_t dcc_badTiming   = 0;

/* ---------------------------------------------------------------------------
   🟦 DCC2CAN — ÉTAT LOGIQUE DU BOOSTER
--------------------------------------------------------------------------- */

volatile BoosterRuntimeState g_state = {
    { DCC_EVT_BIT, 0, 0, 0 },   // lastEvent
    BSTATE_RUNNING,             // status
    0                           // lastEventTime
};

StaticSemaphore_t gStateUpdateMutexBuffer;
SemaphoreHandle_t gStateUpdateMutex = nullptr;

uint32_t recoveryStartTime = 0;

/* ---------------------------------------------------------------------------
   🟩 ERM — STOP / SAVE / RESTART
--------------------------------------------------------------------------- */

volatile uint8_t g_stopState    = 0;
volatile uint8_t g_saveState    = 0;
volatile uint8_t g_restartState = 0;

/* ---------------------------------------------------------------------------
   🟩 ERM — Mode test
--------------------------------------------------------------------------- */

bool g_isTestMode = false;

/* ---------------------------------------------------------------------------
   🟩 ERM — Gestion des satellites
--------------------------------------------------------------------------- */

ERM_SatManager satManager;
volatile uint8_t g_satOnlineCount = 0;
volatile uint8_t g_lastSatId      = 0;

/* ---------------------------------------------------------------------------
   🟩 ERM — CAN : tableau de bus
--------------------------------------------------------------------------- */

CanBus* CAN[2] = { nullptr, nullptr };
volatile uint32_t g_lastCanError = 0;

/* ---------------------------------------------------------------------------
   🟩 ERM — Identité maître
--------------------------------------------------------------------------- */

uint16_t idMain = 254;

/* ---------------------------------------------------------------------------
   🟩 ERM — Logs
--------------------------------------------------------------------------- */

String g_lastLogMessage = "";

/* ---------------------------------------------------------------------------
   🟩 ERM — Services globaux
--------------------------------------------------------------------------- */

ERM_CanService canService;
ERM_WebHandler webHandler(&canService);

/* ---------------------------------------------------------------------------
   🟥 ERS — PARAMÈTRES DU WATCHDOG
--------------------------------------------------------------------------- */

uint32_t ERS_TIMEOUT_MS    = 1500; // 1.5 s
uint32_t ERS_SUP_PERIOD_MS = 100;  // supervision toutes les 100 ms
uint32_t ERS_RX_PERIOD_MS  = 5;    // lecture CAN toutes les 5 ms

/* ---------------------------------------------------------------------------
   🟥 ERS — TABLE DES HEARTBEATS
--------------------------------------------------------------------------- */

uint32_t ers_lastHeartbeat[NB_SAT] = {0};

StaticSemaphore_t ersHeartbeatMutexBuffer;
SemaphoreHandle_t ersHeartbeatMutex = nullptr;

/* ---------------------------------------------------------------------------
   🟥 ERS — COMPTEURS ET STATISTIQUES
--------------------------------------------------------------------------- */

volatile uint32_t ers_heartbeatCount = 0;
volatile uint32_t ers_timeoutCount   = 0;
volatile uint32_t ers_recoveryCount  = 0;

/* ---------------------------------------------------------------------------
   🟥 ERS — ÉTAT INTERNE DU WATCHDOG
--------------------------------------------------------------------------- */

volatile uint8_t  ers_onlineCount = 0;
volatile uint16_t ers_lastSatId   = 0;

bool ers_enabled = true;

/* ---------------------------------------------------------------------------
   🟥 ERS — TIMERS INTERNES
--------------------------------------------------------------------------- */

uint32_t ers_lastSupervisionTime = 0;
uint32_t ers_lastStateChangeTime = 0;
