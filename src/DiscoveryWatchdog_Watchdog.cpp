/*
DiscoveryWatchdog_Watchdog.cpp — Version Discovery 2026

🎯 Rôle
Module central du Watchdog Master Discovery 2026.
Il surveille l’activité des satellites via leurs heartbeat CAN (0x200)
et déclenche automatiquement un STOP global (0x201) lorsqu’un satellite
devient silencieux.

📌 Fonctionnement
- DiscoveryWatchdog_init() : reset interne
- DiscoveryWatchdog_registerHeartbeat() : met à jour SatManager
- DiscoveryWatchdog_supervise() :
      • analyse les timeouts via SatManager
      • déclenche STOP global si un SA est offline

📌 Particularités
- STOP Discovery 2026 = ID 0x201, 11 bits, len = 0
- Heartbeat SA = ID 0x200, 11 bits, data = [ID_H, ID_L]
- Utilise SatManager comme source de vérité
*/

#include "DiscoveryWatchdog_Watchdog.h"
#include "DiscoveryMaster_SatManager.h"
#include "DiscoveryMaster_CanService.h"
#include "ProtocolCAN.h" // <-- nécessaire pour PROTOCOLCAN_ID_STOP
#include "CanMsg.h"      // <-- nécessaire pour CanMsg

extern DiscoveryMaster_SatManager satManager;
extern DiscoveryMaster_CanService canService;

// ---------------------------------------------------------------------------
// Initialisation
// ---------------------------------------------------------------------------
void DiscoveryWatchdog_init()
{
    // Rien à faire : SatManager gère déjà lastSeen/online
}

// ---------------------------------------------------------------------------
// Mise à jour heartbeat (appelé par TaskRx)
// ---------------------------------------------------------------------------
void DiscoveryWatchdog_registerHeartbeat(uint16_t satId)
{
    satManager.updateHeartbeat(satId);
}

// ---------------------------------------------------------------------------
// STOP global Discovery 2026
// ---------------------------------------------------------------------------
void DiscoveryWatchdog_triggerEmergencyStop()
{
    // Construction d’une trame 11 bits STOP global
    CanMsg msg(uint16_t(PROTOCOLCAN_ID_STOP), {}); // ID = 0x201, DLC = 0

    canService.sendMessage(msg);
    Serial.println("[WD] STOP global envoyé !");
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
        Serial.printf("[WD] Satellite %d OFFLINE → STOP global\n", offlineId);
        DiscoveryWatchdog_triggerEmergencyStop();
    }
}
