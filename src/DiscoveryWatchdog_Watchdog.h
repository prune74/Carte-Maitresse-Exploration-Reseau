#pragma once
#include <Arduino.h>
#include "DiscoveryMaster_Config.h"
#include "DiscoveryMaster_CanService.h"

/*
SAWatchdog.h — Version Discovery 2026

🎯 Rôle
Définition de l’API publique du module Watchdog Master Discovery 2026.
Surveille les satellites via leurs heartbeat CAN et déclenche un STOP global
en cas de silence prolongé.

📌 Fonctionnement
- Paramètres failsafe (timeouts, périodes de supervision).
- API principale :
    • DiscoveryWatchdog_init()
    • DiscoveryWatchdog_registerHeartbeat(uint16_t satId)
    • DiscoveryWatchdog_triggerEmergencyStop()   → STOP global Discovery
    • DiscoveryWatchdog_supervise()              → analyse des timeouts
- Tâches FreeRTOS :
    • DiscoveryWatchdog_TaskRx
    • DiscoveryWatchdog_TaskSupervision
*/

// Paramètres failsafe
static const uint32_t WD_TIMEOUT_MS   = 500; // délai max sans heartbeat
static const uint32_t WD_PERIOD_MS    = 250; // période de supervision
static const uint32_t WD_RX_PERIOD_MS = 1;   // période de lecture CAN

// API Watchdog
void DiscoveryWatchdog_init();
void DiscoveryWatchdog_registerHeartbeat(uint16_t satId);
void DiscoveryWatchdog_triggerEmergencyStop();   // STOP global (plus de satId)
void DiscoveryWatchdog_supervise();
void DiscoveryWatchdog_begin();

// Tâches FreeRTOS
void DiscoveryWatchdog_TaskRx(void *pv);
void DiscoveryWatchdog_TaskSupervision(void *pv);
