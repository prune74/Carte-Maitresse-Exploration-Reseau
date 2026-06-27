#pragma once
#include <Arduino.h>
#include "ERM_Config.h"
#include "ERM_CanService.h"

/*
 * ERS_Watchdog.h
 *
 * 🎯 Rôle
 * Définition de l’API publique du module de Surveillance ERS.
 *
 * Le module ERS surveille l’activité des Canton Controllers via leurs heartbeats CAN
 * (ID 0x200) et déclenche automatiquement un STOP global (ID 0x201) lorsqu’un
 * Canton Controller devient silencieux.
 *
 * 📌 Fonctionnement
 * - Paramètres failsafe (timeouts, périodes de supervision) → centralisés
 *   dans Variables.h
 * - API principale :
 *      • ERS_init()
 *      • ERS_registerHeartbeat(uint16_t ccId)
 *      • ERS_triggerEmergencyStop()
 *      • ERS_supervise()
 *      • ERS_begin()
 * - Tâches FreeRTOS :
 *      • ERS_TaskRx
 *      • ERS_TaskSupervision
 */

// ---------------------------------------------------------------------------
// API Watchdog ERS
// ---------------------------------------------------------------------------
void ERS_init();
void ERS_registerHeartbeat(uint16_t ccId);
void ERS_triggerEmergencyStop(); // STOP global (ID 0x201)
void ERS_triggerCcOffline(uint16_t offlineId);
void ERS_supervise();
void ERS_begin();

// ---------------------------------------------------------------------------
// Tâches FreeRTOS
// ---------------------------------------------------------------------------
void ERS_TaskRx(void *pv);
void ERS_TaskSupervision(void *pv);
