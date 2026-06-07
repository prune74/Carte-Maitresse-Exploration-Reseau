/*
DiscoveryWatchdog_Watchdog.cpp — Version Discovery 2026

🎯 Rôle
Module central du Watchdog Master Discovery 2026.
Il surveille l’activité des satellites via leurs heartbeat CAN (0x200)
et déclenche automatiquement un STOP global (0x201) lorsqu’un satellite
devient silencieux.
*/

#include "DiscoveryWatchdog_Watchdog.h"
#include "DiscoveryMaster_SatManager.h"
#include "DiscoveryMaster_CanService.h"
#include "ProtocolCAN.h"
#include "CanMsg.h"
#include "Debug.h"

extern DiscoveryMaster_SatManager satManager;
extern DiscoveryMaster_CanService canService;

// ---------------------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------------------
void DiscoveryWatchdog_init()
{
    LOG_INFO("Watchdog → init interne OK");
    // Rien d’autre : SatManager gère déjà lastSeen/online
}

// ---------------------------------------------------------------------------
// Mise à jour heartbeat (appelé par TaskRx)
// ---------------------------------------------------------------------------
void DiscoveryWatchdog_registerHeartbeat(uint16_t satId)
{
    satManager.updateHeartbeat(satId);
    LOG_VERBOSE("Watchdog → heartbeat reçu de %u", satId);
}

// ---------------------------------------------------------------------------
// STOP global Discovery 2026
// ---------------------------------------------------------------------------
void DiscoveryWatchdog_triggerEmergencyStop()
{
    // Construction d’une trame 11 bits STOP global
    CanMsg msg(uint16_t(PROTOCOLCAN_ID_STOP), {}); // ID = 0x201, DLC = 0

    canService.sendMessage(msg);
    LOG_WARN("[WD] STOP global envoyé !");
}

// ---------------------------------------------------------------------------
// Supervision (appelé par TaskSupervision)
// ---------------------------------------------------------------------------
void DiscoveryWatchdog_supervise()
{
    // Mise à jour des états online/offline
    satManager.checkTimeouts(WD_TIMEOUT_MS);

    // Vérification d’un satellite offline
    uint16_t offlineId = 0;

    if (satManager.hasOfflineSatellite(offlineId))
    {
        LOG_ERROR("[WD] Satellite %u OFFLINE → STOP global", offlineId);
        DiscoveryWatchdog_triggerEmergencyStop();
    }
    else
    {
        LOG_VERBOSE("Watchdog → aucun satellite offline");
    }
}
