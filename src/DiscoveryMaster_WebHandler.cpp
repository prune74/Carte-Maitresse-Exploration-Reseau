#include "DiscoveryMaster_WebHandler.h"
#include "DiscoveryMaster_Settings.h"
#include "Debug.h"

extern DiscoveryMaster_SatManager satManager;
extern DiscoveryMaster_CanService canService;

// ---------------------------------------------------------------------------
// CONSTRUCTEUR
// ---------------------------------------------------------------------------
DiscoveryMaster_WebHandler::DiscoveryMaster_WebHandler(DiscoveryMaster_CanService *canService)
    : _server(nullptr), _ws(nullptr), _can(canService)
{
}

// ---------------------------------------------------------------------------
// INITIALISATION
// ---------------------------------------------------------------------------
void DiscoveryMaster_WebHandler::init(uint16_t webPort)
{
    LOG_INFO("WebHandler → initialisation (port %u)", webPort);

    _server = new AsyncWebServer(webPort);
    _ws     = new AsyncWebSocket("/ws");

    _ws->onEvent(std::bind(&DiscoveryMaster_WebHandler::_WsEvent,
                           this,
                           std::placeholders::_1,
                           std::placeholders::_2,
                           std::placeholders::_3,
                           std::placeholders::_4,
                           std::placeholders::_5,
                           std::placeholders::_6));

    route();

    _server->addHandler(_ws);
    _server->begin();

    LOG_INFO("WebHandler → serveur HTTP/WebSocket démarré");
}

void DiscoveryMaster_WebHandler::loop()
{
    _ws->cleanupClients();
}

// ---------------------------------------------------------------------------
// PUSH WEBSOCKET AUTOMATIQUE
// ---------------------------------------------------------------------------
void DiscoveryMaster_WebHandler::pushStatus()
{
    StaticJsonDocument<1024> doc;

    // --- État global ---
    doc["wifi_on"]       = DiscoveryMaster_Settings::WIFI_ON;
    doc["discovery_on"]  = DiscoveryMaster_Settings::DISCOVERY_ON;
    doc["track_profile"] = DiscoveryMaster_Settings::track_profile;

    // --- État CAN ---
    doc["can_ok"]     = canService.isCanOK();
    doc["can_last_ms"] = canService.lastRxAgeMs();

    // --- Liste des satellites ---
    JsonArray satsJson = doc.createNestedArray("sats");
    const DiscoveryMaster_Satellite *sats = satManager.getAll();

    for (int i = 0; i < NB_SAT; i++)
    {
        const auto &s = sats[i];
        if (s.id != NO_ID)
        {
            JsonObject o = satsJson.createNestedObject();
            o["id"]       = s.id;
            o["online"]   = s.online;
            o["lastSeen"] = s.lastSeen;
        }
    }

    // --- Envoi WebSocket ---
    String json;
    serializeJson(doc, json);
    _ws->textAll(json);

    LOG_VERBOSE("WebHandler → pushStatus (%u satellites)", satsJson.size());
}

// ---------------------------------------------------------------------------
// MESSAGE TEXTE SIMPLE
// ---------------------------------------------------------------------------
void DiscoveryMaster_WebHandler::handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
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
void DiscoveryMaster_WebHandler::_WsEvent(AsyncWebSocket *server,
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

            DiscoveryMaster_Settings::WIFI_ON = on;
            _can->sendWifiOnOff(on);
            DiscoveryMaster_Settings::writeFile();
        }

        // -------------------------------------------------------------------
        // DISCOVERY ON/OFF
        // -------------------------------------------------------------------
        if (message.indexOf("discovery_on") >= 0)
        {
            bool on = doc["discovery_on"].as<bool>();
            LOG_INFO("[WEB] DISCOVERY_ON = %s", on ? "true" : "false");

            DiscoveryMaster_Settings::DISCOVERY_ON = on;
            _can->sendDiscoveryOnOff(on);
            DiscoveryMaster_Settings::writeFile();
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
        // PROFIL VOIE (N / HO)
        // -------------------------------------------------------------------
        if (message.indexOf("set_profile") >= 0)
        {
            uint8_t profile = doc["value"] | 0;
            LOG_INFO("[WEB] Profil voie → %u", profile);

            DiscoveryMaster_Settings::track_profile = profile;
            DiscoveryMaster_Settings::writeFile();

            _can->sendTrackProfile(profile);
            pushStatus();
        }

        // -------------------------------------------------------------------
        // CLEAR STOP GLOBAL (0x202)
        // -------------------------------------------------------------------
        if (message.indexOf("clear_stop") >= 0)
        {
            CANMessage msg;
            msg.id  = PROTOCOLCAN_ID_CLEAR_STOP;
            msg.ext = false;
            msg.len = 0;

            _can->sendMessage(msg);
            LOG_INFO("[WEB] CLEAR STOP envoyé");
        }

        // -------------------------------------------------------------------
        // STOP GLOBAL (0x201)
        // -------------------------------------------------------------------
        else if (message.indexOf("stop") >= 0)
        {
            CANMessage msg;
            msg.id  = PROTOCOLCAN_ID_STOP;
            msg.ext = false;
            msg.len = 0;

            _can->sendMessage(msg);
            LOG_WARN("[WEB] STOP global envoyé");
        }
    }
    break;
    }
}

void DiscoveryMaster_WebHandler::notifyClients()
{
    _ws->textAll(String("ok"));
}

// ---------------------------------------------------------------------------
// ROUTES HTTP
// ---------------------------------------------------------------------------
void DiscoveryMaster_WebHandler::route()
{
    LOG_INFO("WebHandler → configuration des routes HTTP");

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
