#include "ExplorationReseau_Maitre_WebHandler.h"
#include "ExplorationReseau_Maitre_Settings.h"
#include "ExplorationReseau_Maitre_SatManager.h"
#include "ExplorationReseau_Maitre_CanService.h"
#include "ExplorationReseau_Maitre_Config.h"
#include "Debug.h"

// Instances externes
extern ERM_SatManager satManager;
extern ERM_CanService canService;
extern bool g_isTestMode;

/*
 * ExplorationReseau_Maitre_WebHandler.cpp
 *
 * 🎯 Rôle
 * Interface Web + WebSocket de la Carte Maîtresse ERM.
 *
 * Ce module assure :
 *   • le serveur HTTP (fichiers statiques)
 *   • le WebSocket bidirectionnel
 *   • la réception des commandes UI
 *   • la diffusion de l’état complet du système
 */

// ---------------------------------------------------------------------------
// CONSTRUCTEUR
// ---------------------------------------------------------------------------
ERM_WebHandler::ERM_WebHandler(ERM_CanService *canService)
    : _server(nullptr), _ws(nullptr), _can(canService)
{
}

// ---------------------------------------------------------------------------
// INITIALISATION
// ---------------------------------------------------------------------------
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

// ---------------------------------------------------------------------------
// PUSH WEBSOCKET AUTOMATIQUE
// ---------------------------------------------------------------------------
void ERM_WebHandler::pushStatus()
{
    StaticJsonDocument<1024> doc;

    // --- État global ---
    doc["wifi_on"] = ERM_Settings::WIFI_ON;
    doc["exploration_on"] = ERM_Settings::EXPLORATION_ON;
    doc["track_profile"] = ERM_Settings::track_profile;
    doc["mode_test"] = ERM_Settings::MODE_TEST;

    // --- État CAN ---
    doc["can_ok"] = canService.isCanOK();
    doc["can_last_ms"] = canService.lastRxAgeMs();

    // --- Liste des satellites ---
    JsonArray satsJson = doc.createNestedArray("sats");

    for (const auto *it = satManager.satBegin(); it != satManager.satEnd(); ++it)
    {
        const auto &s = *it;
        if (s.id != NO_ID)
        {
            JsonObject o = satsJson.createNestedObject();
            o["id"] = s.id;
            o["online"] = s.online;
            o["lastSeen"] = s.lastSeen;
        }
    }

    // --- Envoi WebSocket ---
    String json;
    serializeJson(doc, json);
    _ws->textAll(json);

    LOG_VERBOSE("ERM_WebHandler → pushStatus (%u satellites)", satsJson.size());
}

// ---------------------------------------------------------------------------
// MESSAGE TEXTE SIMPLE
// ---------------------------------------------------------------------------
void ERM_WebHandler::ERM_handleSimpleMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo *)arg;

    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0;

        if (strcmp((char *)data, "toggle") == 0)
            notifyClients();
    }
}

// ---------------------------------------------------------------------------
// ÉVÉNEMENTS WEBSOCKET
// ---------------------------------------------------------------------------
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
        break;

    case WS_EVT_DISCONNECT:
        LOG_INFO("WebSocket → client #%u déconnecté", client->id());
        break;

    case WS_EVT_DATA:
    {
        StaticJsonDocument<1024> doc;
        DeserializationError error = deserializeJson(doc, data);

        if (error)
        {
            LOG_WARN("WebSocket → erreur JSON");
            return;
        }

        String message = (char *)data;
        LOG_VERBOSE("WebSocket RX → %s", message.c_str());

        // -------------------------------------------------------------------
        // WIFI ON/OFF
        // -------------------------------------------------------------------
        if (message.indexOf("wifi_on") >= 0)
        {
            bool on = doc["wifi_on"].as<bool>();
            LOG_INFO("[WEB] WIFI_ON = %s", on ? "true" : "false");

            ERM_Settings::WIFI_ON = on;
            _can->sendWifiOnOff(on);
            ERM_Settings::writeFile();
        }

        // -------------------------------------------------------------------
        // EXPLORATION ON/OFF
        // -------------------------------------------------------------------
        if (message.indexOf("exploration_on") >= 0)
        {
            bool on = doc["exploration_on"].as<bool>();
            LOG_INFO("[WEB] EXPLORATION_ON = %s", on ? "true" : "false");

            ERM_Settings::EXPLORATION_ON = on;
            _can->sendDiscoveryOnOff(on);
            ERM_Settings::writeFile();
        }

        // -------------------------------------------------------------------
        // MODE TEST
        // -------------------------------------------------------------------
        if (message.indexOf("mode_test") >= 0)
        {
            bool on = doc["mode_test"].as<bool>();
            LOG_WARN("[WEB] MODE_TEST = %s", on ? "true" : "false");

            ERM_Settings::MODE_TEST = on;
            ERM_Settings::writeFile();

            g_isTestMode = on;

            pushStatus();
        }

        // -------------------------------------------------------------------
        // SAVE
        // -------------------------------------------------------------------
        if (message.indexOf("save") >= 0)
        {
            LOG_INFO("[WEB] SAVE demandé");
            _can->sendSaveAll();
        }

        // -------------------------------------------------------------------
        // RESTART
        // -------------------------------------------------------------------
        if (message.indexOf("restartEsp") >= 0)
        {
            LOG_WARN("[WEB] RESTART demandé");
            _can->sendRestartAll();
        }

        // -------------------------------------------------------------------
        // PROFIL VOIE
        // -------------------------------------------------------------------
        if (message.indexOf("set_profile") >= 0)
        {
            uint8_t profile = doc["value"] | 0;
            LOG_INFO("[WEB] Profil voie → %u", profile);

            ERM_Settings::track_profile = profile;
            ERM_Settings::writeFile();

            _can->sendTrackProfile(profile);
            pushStatus();
        }

        // -------------------------------------------------------------------
        // CLEAR STOP GLOBAL
        // -------------------------------------------------------------------
        if (message.indexOf("clear_stop") >= 0)
        {
            CanMsg msg(uint16_t(PROTOCOLCAN_ID_CLEAR_STOP), {});
            _can->sendMessage(msg);

            LOG_INFO("[WEB] CLEAR STOP envoyé");
        }

        // -------------------------------------------------------------------
        // STOP GLOBAL
        // -------------------------------------------------------------------
        else if (message.indexOf("stop") >= 0)
        {
            CanMsg msg(uint16_t(PROTOCOLCAN_ID_STOP), {});
            _can->sendMessage(msg);

            LOG_WARN("[WEB] STOP global envoyé");
        }
    }
    break;
    }
}

void ERM_WebHandler::notifyClients()
{
    _ws->textAll(String("ok"));
}

// ---------------------------------------------------------------------------
// ROUTES HTTP
// ---------------------------------------------------------------------------
void ERM_WebHandler::ERM_route()
{
    LOG_INFO("ERM_WebHandler → configuration des routes HTTP");

    _server->on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/index.html", "text/html"); });

    _server->on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/w3.css", "text/css"); });

    _server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/style.css", "text/css"); });

    _server->on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/script.js", "text/javascript"); });

    _server->on("/settings.json", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/settings.json", "text/json"); });

    _server->on("/favicon.png", HTTP_GET, [](AsyncWebServerRequest *request)
                { request->send(SPIFFS, "/favicon.png", "image/png"); });

    _server->onNotFound([](AsyncWebServerRequest *request)
                        {
                            LOG_WARN("HTTP 404 → %s", request->url().c_str());
                            request->send(404); });
}
