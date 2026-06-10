/*
 * ExplorationReseau_Surveillance_Watchdog.cpp
 *
 * 🎯 Rôle
 * Module central de la Surveillance ERS.
 *
 * Il surveille l’activité des satellites via leurs heartbeats CAN (0x200)
 * et déclenche automatiquement un STOP global (0x201) lorsqu’un satellite
 * devient silencieux.
 */

#include "ExplorationReseau_Surveillance_Watchdog.h"
#include "Variables.h"
#include "ExplorationReseau_Maitre_SatManager.h"
#include "ExplorationReseau_Maitre_CanService.h"
#include "ProtocolCAN.h"
#include "CanMsg.h"
#include "Debug.h"

// Instances externes
extern ERM_SatManager satManager;
extern ERM_CanService canService;

/* ---------------------------------------------------------------------------
   INITIALISATION
--------------------------------------------------------------------------- */
void ERS_init()
{
    LOG_INFO("ERS → init interne OK");

    // Création du mutex si nécessaire
    if (ersHeartbeatMutex == nullptr)
    {
        ersHeartbeatMutex = xSemaphoreCreateMutexStatic(&ersHeartbeatMutexBuffer);
    }

    ers_enabled = true;
    ers_onlineCount = 0;
    ers_lastSupervisionTime = millis();
}

/* ---------------------------------------------------------------------------
   MISE À JOUR HEARTBEAT
--------------------------------------------------------------------------- */
void ERS_registerHeartbeat(uint16_t satId)
{
    if (!ers_enabled)
        return;

    // Mise à jour SatManager (online/offline)
    satManager.updateHeartbeat(satId);

    // Mise à jour table interne ERS
    if (xSemaphoreTake(ersHeartbeatMutex, 0) == pdTRUE)
    {
        ers_lastHeartbeat[satId] = millis();
        xSemaphoreGive(ersHeartbeatMutex);
    }

    ers_heartbeatCount++;
    ers_lastSatId = satId;

    LOG_VERBOSE("ERS → heartbeat reçu de %u", satId);
}

/* ---------------------------------------------------------------------------
   STOP GLOBAL
--------------------------------------------------------------------------- */
void ERS_triggerEmergencyStop()
{
    CanMsg msg(uint16_t(PROTOCOLCAN_ID_STOP), {}); // ID = 0x201, DLC = 0

    canService.sendMessage(msg);
    LOG_WARN("[ERS] STOP global envoyé !");
}

/* ---------------------------------------------------------------------------
   SUPERVISION
--------------------------------------------------------------------------- */
void ERS_supervise()
{
    if (!ers_enabled)
        return;

    ers_lastSupervisionTime = millis();

    // Mise à jour des états online/offline
    satManager.checkTimeouts(ERS_TIMEOUT_MS);

    uint16_t offlineId = 0;

    if (satManager.hasOfflineSatellite(offlineId))
    {
        ers_timeoutCount++;

        LOG_ERROR("[ERS] Satellite %u OFFLINE → STOP global", offlineId);
        ERS_triggerEmergencyStop();
    }
    else
    {
        LOG_VERBOSE("ERS → aucun satellite offline");
    }
}
