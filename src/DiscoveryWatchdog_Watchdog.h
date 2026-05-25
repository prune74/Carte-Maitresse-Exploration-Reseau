#pragma once
#include <Arduino.h>
#include "DiscoveryMaster_Config.h"
#include "DiscoveryMaster_CanService.h"

/*
SAWatchdog.h

🎯 Rôle
Définition de l’API publique du module Watchdog Discovery 2026.
Ce header expose les constantes, structures et fonctions nécessaires à la
surveillance des satellites via leurs heartbeat CAN, ainsi que les tâches
FreeRTOS associées.

📌 Fonctionnement
- Déclare les paramètres failsafe (timeouts, périodes de supervision).
- Déclare le tableau global des timestamps de heartbeat.
- Expose les fonctions principales :
    • DiscoveryWatchdog_init()                → initialise la table des heartbeat
    • DiscoveryWatchdog_registerHeartbeat()   → met à jour le timestamp d’un satellite
    • DiscoveryWatchdog_triggerEmergencyStop()→ envoie un STOP d’urgence Discovery
    • DiscoveryWatchdog_supervise()           → analyse les timeouts
- Déclare les tâches FreeRTOS :
    • DiscoveryWatchdog_TaskRx                → réception des heartbeat via CAN
    • DiscoveryWatchdog_TaskSupervision       → supervision et STOP d’urgence

📌 Particularités
- Le Watchdog est totalement indépendant du module SAMain.
- Le module utilise une API CAN publique (sendMessage) pour garantir
  l’isolation entre protocole Discovery et supervision.
- Conçu pour être modulaire : peut être activé/désactivé ou réutilisé dans
  d’autres projets Discovery 2026.
*/

// Nombre maximum de satellites Discovery 2026
static const uint16_t WD_MAX_SAT = 250;

// Tableau des timestamps de heartbeat
extern volatile uint32_t wd_lastHeartbeat[WD_MAX_SAT];

// Paramètres failsafe
static const uint32_t WD_TIMEOUT_MS = 500; // délai max sans heartbeat
static const uint32_t WD_PERIOD_MS = 250;  // période de supervision
static const uint32_t WD_RX_PERIOD_MS = 1; // période de lecture CAN

// API Watchdog
void DiscoveryWatchdog_init();
void DiscoveryWatchdog_registerHeartbeat(uint16_t satId);
void DiscoveryWatchdog_triggerEmergencyStop(uint16_t satId);
void DiscoveryWatchdog_supervise();
void DiscoveryWatchdog_begin();

// Tâches FreeRTOS
void DiscoveryWatchdog_TaskRx(void *pv);
void DiscoveryWatchdog_TaskSupervision(void *pv);
