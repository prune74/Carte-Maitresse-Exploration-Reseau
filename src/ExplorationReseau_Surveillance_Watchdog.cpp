/*
 * ExplorationReseau_Surveillance_Watchdog.cpp
 *
 * 🎯 Rôle
 * Module central de la Surveillance ERS.
 *
 * Il surveille l’activité des satellites via leurs heartbeats CAN (ID 0x200)
 * et déclenche automatiquement un STOP global (ID 0x201) lorsqu’un satellite
 * devient silencieux.
 *
 * Le Watchdog ERS repose sur :
 *   • SatManager (online/offline)
 *   • une table interne des timestamps de heartbeats
 *   • deux tâches FreeRTOS :
 *        - ERS_TaskRx (prio 4) : réception des heartbeats
 *        - ERS_TaskSupervision (prio 2) : analyse des timeouts
 */

#include "ExplorationReseau_Surveillance_Watchdog.h"
#include "ExplorationReseau_Maitre_Pins.h"
#include "ExplorationReseau_Protocol.h"
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
 * 🧩 INITIALISATION
 * ------------------------------------------------------------------------- */
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

    // LED CC OFFLINE
    pinMode(PIN_LED_CC_OFFLINE, OUTPUT);
    digitalWrite(PIN_LED_CC_OFFLINE, LOW);
}

/* ---------------------------------------------------------------------------
 * 📡 MISE À JOUR HEARTBEAT
 *
 * Appelée par ERS_TaskRx à chaque réception d’un heartbeat (ID 0x200).
 * ------------------------------------------------------------------------- */
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

    // Log sécurisé : jamais actif en mode réel
    LOG_CRITICAL_DCC("ERS → heartbeat reçu de %u", satId);
}

/* ---------------------------------------------------------------------------
 * 🛑 STOP GLOBAL
 *
 * Envoie une trame CAN STOP (ID 0x201).
 * ------------------------------------------------------------------------- */
void ERS_triggerEmergencyStop()
{
    CanMsg msg(uint16_t(PROTOCOLCAN_ID_STOP), {}); // ID = 0x201, DLC = 0

    canService.sendMessage(msg);
    LOG_WARN("[ERS] STOP global envoyé !");
}

/* ---------------------------------------------------------------------------
 * 🟥 CC OFFLINE
 *
 * Envoie une trame CAN CMD_CC_OFFLINE (commande 0xC0) contenant l’ID
 * du CC devenu hors ligne.
 * ------------------------------------------------------------------------- */
void ERS_triggerCcOffline(uint16_t offlineId)
{
    // Payload = [ID_H][ID_L]
    uint8_t data[2] = {
        uint8_t(offlineId >> 8),
        uint8_t(offlineId & 0xFF)};

    // Construction de la trame 29 bits
    CanMsg msg = ProtocolCAN::makeMsg(
        2,              // priorité
        CMD_CC_OFFLINE, // commande
        false,          // pas une réponse
        idMain,         // source = ERM
        {data[0], data[1]});

    canService.sendMessage(msg);
    LOG_WARN("[ERS] CC %u OFFLINE → notification envoyée !", offlineId);
}

/* ---------------------------------------------------------------------------
 * 🛡️ SUPERVISION
 *
 * Appelée par ERS_TaskSupervision (prio 2).
 * Analyse les timeouts et déclenche un STOP global si un satellite est OFFLINE.
 * ------------------------------------------------------------------------- */
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

        LOG_ERROR("[ERS] CC %u OFFLINE → STOP global + notification CC", offlineId);

        ERS_triggerEmergencyStop();             // STOP global
        ERS_triggerCcOffline(offlineId);        // Notification aux autres CC
        digitalWrite(PIN_LED_CC_OFFLINE, HIGH); // LED CC OFFLINE
    }
    else
    {
        LOG_INFO("ERS → aucun satellite offline");
        digitalWrite(PIN_LED_CC_OFFLINE, LOW); // LED CC OFFLINE éteinte
    }
}
