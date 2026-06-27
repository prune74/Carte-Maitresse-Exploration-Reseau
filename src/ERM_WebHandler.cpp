/*
 * ERM_WebHandler.cpp
 *
 * 🎯 Rôle
 * Gestion du serveur HTTP + WebSocket de la Carte Maîtresse.
 *
 * Ce module assure :
 *   • la communication WebSocket avec l’interface utilisateur
 *   • l’envoi d’états (CAN, Canton Controllers, paramètres…)
 *   • la réception des commandes Web (WIFI, EXPLORATION, STOP, CLEAR STOP…)
 *
 * Depuis la refonte STOP :
 *   → Toute la logique STOP/CLEAR STOP est centralisée dans ERM_StopService.
 *   → WebHandler ne construit plus de trames CAN STOP/CLEAR STOP.
 */

#include "ERM_WebHandler.h"
#include "ERM_Settings.h"
#include "ERM_CC_Manager.h"
#include "ERM_CanService.h"
#include "ERM_Config.h"
#include "ERM_StopService.h"
#include "Variables.h"

#include "Debug.h"

// Instances externes
extern ERM_CC_Manager CC_Manager;
extern ERM_CanService canService;
extern bool g_isTestMode;

// États globaux (UI)
extern volatile uint8_t g_stopState;
extern volatile uint8_t g_saveState;
extern volatile uint8_t g_restartState;

/* ---------------------------------------------------------------------------
 * 🧩 CONSTRUCTEUR
 * ------------------------------------------------------------------------- */
ERM_WebHandler::ERM_WebHandler(ERM_CanService *canService)
    : _server(nullptr), _ws(nullptr), _can(canService)
{
}

/* ---------------------------------------------------------------------------
 * 🚀 INITIALISATION DU SERVEUR WEB + WEBSOCKET
 * ------------------------------------------------------------------------- */
void ERM_WebHandler::init(uint16_t webPort)
{
    LOG_INFO("ERM_WebHandler → initialisation (port %u)", webPort);

    _server = new AsyncWebServer(webPort);
    _ws = new AsyncWebSocket("/ws");

    _ws->onEvent(std::bind(&ERM_WebHandler::ERM_wsEvent,
                           this,
                           std::placeholders::_1,
                           std::placeholders::_2,
                           std::placeholders::_3,
                           std::placeholders::_4,
                           std::placeholders::_5,
                           std::placeholders::_6));

    ERM_route();

    _server->addHandler(_ws);
    _server->begin();

    LOG_INFO("ERM_WebHandler → serveur HTTP/WebSocket démarré");
}

void ERM_WebHandler::loop()
{
    _ws->cleanupClients();
}

/* ---------------------------------------------------------------------------
 * 📡 PUSH WEBSOCKET AUTOMATIQUE (état complet)
 * ------------------------------------------------------------------------- */
void ERM_WebHandler::pushStatus()
{
    StaticJsonDocument<2048> doc;

    // --- État global ---
    doc["wifi_on"] = ERM_Settings::WIFI_ON;
    doc["exploration_on"] = ERM_Settings::EXPLORATION_ON;
    doc["track_profile"] = ERM_Settings::track_profile;
    doc["mode_test"] = ERM_Settings::MODE_TEST;

    // --- États STOP / SAVE / RESTART ---
    doc["stop_state"] = g_stopState;
    doc["save_state"] = g_saveState;
    doc["restart_state"] = g_restartState;

    // --- État CAN ---
    doc["can_ok"] = canService.isCanOK();
    doc["can_last_ms"] = canService.lastRxAgeMs();

    // --- Liste des Canton Controllers ---
    JsonArray ccsJson = doc.createNestedArray("ccs");

    for (const auto *it = CC_Manager.ccBegin(); it != CC_Manager.ccEnd(); ++it)
    {
        const auto &s = *it;
        if (s.id != NO_ID)
        {
            JsonObject o = ccsJson.createNestedObject();
            o["id"] = s.id;
            o["online"] = s.online;
            o["lastSeen"] = s.lastSeen;
        }
    }

    // --- Envoi WebSocket ---
    String json;
    serializeJson(doc, json);
    _ws->textAll(json);
}

/* ---------------------------------------------------------------------------
 * 📝 ENVOI D’UN LOG AU DASHBOARD
 * ------------------------------------------------------------------------- */
void ERM_WebHandler::pushLog(const char *type, const char *msg)
{
    StaticJsonDocument<256> doc;
    doc["log"]["type"] = type;
    doc["log"]["msg"] = msg;

    String json;
    serializeJson(doc, json);
    _ws->textAll(json);
}

/* ---------------------------------------------------------------------------
 * 📨 ENVOI D’UNE TRAME CAN AU DASHBOARD
 * ------------------------------------------------------------------------- */
void ERM_WebHandler::pushCanFrame(const CanMsg &msg, const char *type)
{
    StaticJsonDocument<256> doc;

    doc["can_frame"]["type"] = type;
    doc["can_frame"]["time"] = millis();
    doc["can_frame"]["id"] = msg.id;
    doc["can_frame"]["dlc"] = msg.dlc;

    char buffer[64];
    char *p = buffer;
    for (uint8_t i = 0; i < msg.dlc; i++)
        p += sprintf(p, "%02X ", msg.data[i]);

    doc["can_frame"]["data"] = buffer;

    String json;
    serializeJson(doc, json);
    _ws->textAll(json);
}

/* ---------------------------------------------------------------------------
 * 🎧 ÉVÉNEMENTS WEBSOCKET (réception commandes UI)
 * ------------------------------------------------------------------------- */
void ERM_WebHandler::ERM_wsEvent(AsyncWebSocket *server,
                                 AsyncWebSocketClient *client,
                                 AwsEventType type,
                                 void *arg,
                                 uint8_t *data,
                                 size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        LOG_INFO("WebSocket → client #%u connecté (%s)",
                 client->id(),
                 client->remoteIP().toString().c_str());
        pushLog("INFO", "Client Web connecté");
        break;

    case WS_EVT_DISCONNECT:
        LOG_INFO("WebSocket → client #%u déconnecté", client->id());
        pushLog("WARN", "Client Web déconnecté");
        break;

    case WS_EVT_DATA:
    {
        StaticJsonDocument<512> doc;
        if (deserializeJson(doc, data))
        {
            LOG_WARN("WebSocket → erreur JSON");
            pushLog("ERR", "Erreur JSON WebSocket");
            return;
        }

        String message = (char *)data;

        /* -----------------------------------------------------------
         * COMMANDES WEB
         * --------------------------------------------------------- */

        // WIFI
        if (message.indexOf("wifi_on") >= 0)
        {
            bool on = doc["wifi_on"];
            ERM_Settings::WIFI_ON = on;
            _can->sendWifiOnOff(on);
            ERM_Settings::writeFile();
            pushLog("INFO", on ? "WIFI ON" : "WIFI OFF");
            pushStatus();
        }

        // EXPLORATION
        if (message.indexOf("exploration_on") >= 0)
        {
            bool on = doc["exploration_on"];
            ERM_Settings::EXPLORATION_ON = on;
            _can->sendExplorationOnOff(on);
            ERM_Settings::writeFile();
            pushLog("INFO", on ? "EXPLORATION ON" : "EXPLORATION OFF");
            pushStatus();
        }

        // MODE TEST
        if (message.indexOf("mode_test") >= 0)
        {
            bool on = doc["mode_test"];
            ERM_Settings::MODE_TEST = on;
            g_isTestMode = on;
            ERM_Settings::writeFile();
            pushLog("INFO", on ? "MODE TEST ON" : "MODE TEST OFF");
            pushStatus();
        }

        // SAVE
        if (message.indexOf("save") >= 0)
        {
            g_saveState = 0;
            _can->sendSaveAll();
            pushLog("INFO", "SAVE ALL envoyé");
            pushStatus();
        }

        // RESTART
        if (message.indexOf("restartEsp") >= 0)
        {
            g_restartState = 0;
            _can->sendRestartAll();
            pushLog("WARN", "RESTART demandé");
            pushStatus();
        }

        // PROFIL VOIE
        if (message.indexOf("set_profile") >= 0)
        {
            uint8_t profile = doc["value"];
            ERM_Settings::track_profile = profile;
            ERM_Settings::writeFile();
            _can->sendTrackProfile(profile);

            String logMsg = "SET_PROFILE = " + String(profile);
            pushLog("INFO", logMsg.c_str());
            pushStatus();
        }

        /* -----------------------------------------------------------
         * STOP / CLEAR STOP (nouvelle gestion centralisée)
         * --------------------------------------------------------- */

        // CLEAR STOP
        if (message.indexOf("clear_stop") >= 0)
        {
            g_stopState = 0;
            ERM_StopService::clearStop(); // ← centralisé
            pushLog("INFO", "CLEAR STOP demandé (Web)");
            pushStatus();
        }

        // STOP
        else if (message.indexOf("stop") >= 0)
        {
            g_stopState = 1;
            ERM_StopService::triggerStop(); // ← centralisé
            pushLog("WARN", "STOP demandé (Web)");
            pushStatus();
        }
    }
    break;
    }
}

/* ---------------------------------------------------------------------------
 * 🌐 ROUTES HTTP
 * ------------------------------------------------------------------------- */
void ERM_WebHandler::ERM_route()
{
    LOG_INFO("ERM_WebHandler → configuration des routes HTTP");

    _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/index.html", "text/html"); });

    _server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/style.css", "text/css"); });

    _server->on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/script.js", "text/javascript"); });

    _server->on("/favicon.png", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/favicon.png", "image/png"); });

    _server->onNotFound([](AsyncWebServerRequest *request)
                        {
                            LOG_WARN("HTTP 404 → %s", request->url().c_str());
                            request->send(404); });
}
