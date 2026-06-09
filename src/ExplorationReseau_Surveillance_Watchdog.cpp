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
#include "ExplorationReseau_Maitre_SatManager.h"
#include "ExplorationReseau_Maitre_CanService.h"
#include "ProtocolCAN.h"
#include "CanMsg.h"
#include "Debug.h"

// Instances externes
extern ERM_SatManager satManager;
extern ERM_CanService canService;

// ---------------------------------------------------------------------------
// Initialisation interne
// ---------------------------------------------------------------------------
void ERS_init()
{
    LOG_INFO("ERS → init interne OK");
    // Rien d’autre : SatManager gère déjà lastSeen/online
}

// ---------------------------------------------------------------------------
// Mise à jour heartbeat (appelé par ERS_TaskRx)
// ---------------------------------------------------------------------------
void ERS_registerHeartbeat(uint16_t satId)
{
    satManager.updateHeartbeat(satId);
    LOG_VERBOSE("ERS → heartbeat reçu de %u", satId);
}

// ---------------------------------------------------------------------------
// STOP global ERM
// ---------------------------------------------------------------------------
void ERS_triggerEmergencyStop()
{
    // Construction d’une trame 11 bits STOP global
    CanMsg msg(uint16_t(PROTOCOLCAN_ID_STOP), {}); // ID = 0x201, DLC = 0

    canService.sendMessage(msg);
    LOG_WARN("[ERS] STOP global envoyé !");
}

// ---------------------------------------------------------------------------
// Supervision (appelé par ERS_TaskSupervision)
// ---------------------------------------------------------------------------
void ERS_supervise()
{
    // Mise à jour des états online/offline
    satManager.checkTimeouts(ERS_TIMEOUT_MS);

    // Vérification d’un satellite offline
    uint16_t offlineId = 0;

    if (satManager.hasOfflineSatellite(offlineId))
    {
        LOG_ERROR("[ERS] Satellite %u OFFLINE → STOP global", offlineId);
        ERS_triggerEmergencyStop();
    }
    else
    {
        LOG_VERBOSE("ERS → aucun satellite offline");
    }
}
