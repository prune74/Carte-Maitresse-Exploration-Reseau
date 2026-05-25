/*
DiscoveryWatchdog_Watchdog.cpp / SAWatchdog.h

🎯 Rôle
Module central du Watchdog Discovery 2026.
Il surveille l’activité des satellites via leurs heartbeat CAN et déclenche
automatiquement un STOP d’urgence lorsqu’un satellite devient silencieux.

📌 Fonctionnement
- DiscoveryWatchdog_init() initialise la table des timestamps.
- DiscoveryWatchdog_registerHeartbeat() met à jour l’heure du dernier heartbeat reçu.
- DiscoveryWatchdog_supervise() analyse périodiquement les timeouts.
- DiscoveryWatchdog_triggerEmergencyStop() envoie une trame CAN d’arrêt d’urgence
  (format Discovery 29 bits) lorsqu’un satellite dépasse le délai autorisé.

📌 Intégration FreeRTOS
Le module est utilisé par deux tâches dédiées :
    • DiscoveryWatchdog_TaskRx          → réception des heartbeat via CAN
    • DiscoveryWatchdog_TaskSupervision → analyse des timeouts et STOP d’urgence
Ces tâches sont déclarées dans SAWatchdog.h et créées dans DiscoveryWatchdog_main.cpp.

📌 Particularités
- Fonctionne indépendamment du protocole Discovery (pas de décodage de commande).
- Utilise une API publique du driver CAN (sendMessage) pour garantir l’isolation
  entre protocole Discovery et supervision.
- Architecture modulaire : le Watchdog peut être activé ou désactivé
  indépendamment du module SAMain.
- Conçu pour assurer la sécurité du réseau Discovery 2026 en détectant les
  satellites silencieux ou défaillants.
*/

#include "DiscoveryWatchdog_Watchdog.h"

extern DiscoveryMaster_CanService canService;

volatile uint32_t wd_lastHeartbeat[WD_MAX_SAT] = {0};

void DiscoveryWatchdog_init()
{
    for (uint16_t i = 0; i < WD_MAX_SAT; i++)
        wd_lastHeartbeat[i] = 0;
}

void DiscoveryWatchdog_registerHeartbeat(uint16_t satId)
{
    if (satId < WD_MAX_SAT)
        wd_lastHeartbeat[satId] = millis();
}

void DiscoveryWatchdog_triggerEmergencyStop(uint16_t satId)
{
    CANMessage msg;
    msg.id = DISCOVERY_CAN_ID_EMERGENCY_STOP;
    msg.ext = true;
    msg.len = 2;
    msg.data[0] = (satId >> 8) & 0xFF;
    msg.data[1] = satId & 0xFF;

    canService.sendMessage(msg);
}

void DiscoveryWatchdog_supervise()
{
    uint32_t now = millis();

    for (uint16_t id = 1; id < WD_MAX_SAT; id++)
    {
        if (wd_lastHeartbeat[id] == 0)
            continue;

        if (now - wd_lastHeartbeat[id] > WD_TIMEOUT_MS)
        {
            DiscoveryWatchdog_triggerEmergencyStop(id);
            wd_lastHeartbeat[id] = 0;
        }
    }
}
