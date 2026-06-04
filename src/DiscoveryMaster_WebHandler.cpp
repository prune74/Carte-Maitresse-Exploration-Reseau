#include "DiscoveryMaster_WebHandler.h"

extern DiscoveryMaster_SatManager satManager;
extern DiscoveryMaster_CanService canService;

DiscoveryMaster_WebHandler::DiscoveryMaster_WebHandler(DiscoveryMaster_CanService *canService)
    : _server(nullptr), _ws(nullptr), _can(canService)
{
}

void DiscoveryMaster_WebHandler::init(uint16_t webPort)
{
    _server = new AsyncWebServer(webPort);
    _ws = new AsyncWebSocket("/ws");

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
    doc["wifi_on"] = DiscoveryMaster_Settings::WIFI_ON;
    doc["discovery_on"] = DiscoveryMaster_Settings::DISCOVERY_ON;

    // 🔥 Profil voie (N / HO)
    doc["track_profile"] = DiscoveryMaster_Settings::track_profile;

    // --- État CAN (supervision) ---
    doc["can_ok"] = canService.isCanOK();
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
        Serial.printf("Client WebSocket #%u connecté depuis %s\n",
                      client->id(),
                      client->remoteIP().toString().c_str());
        break;

    case WS_EVT_DISCONNECT:
        Serial.printf("Client WebSocket #%u déconnecté\n", client->id());
        break;

    case WS_EVT_DATA:
    {
        StaticJsonDocument<1024> doc1;
        DeserializationError error = deserializeJson(doc1, data);

        if (error)
        {
            Serial.println("Erreur JSON");
            return;
        }

        String message = (char *)data;

        // --- WIFI ON/OFF ---
        if (message.indexOf("wifi_on") >= 0)
        {
            DiscoveryMaster_Settings::WIFI_ON = doc1["wifi_on"].as<bool>();
            _can->sendWifiOnOff(DiscoveryMaster_Settings::WIFI_ON);
            DiscoveryMaster_Settings::writeFile();
        }

        // --- DISCOVERY ON/OFF ---
        if (message.indexOf("discovery_on") >= 0)
        {
            DiscoveryMaster_Settings::DISCOVERY_ON = doc1["discovery_on"].as<bool>();
            _can->sendDiscoveryOnOff(DiscoveryMaster_Settings::DISCOVERY_ON);
            DiscoveryMaster_Settings::writeFile();
        }

        // --- SAVE ---
        if (message.indexOf("save") >= 0)
        {
            _can->sendSaveAll();
        }

        // --- RESTART ---
        if (message.indexOf("restartEsp") >= 0)
        {
            _can->sendRestartAll();
        }

        // -------------------------------------------------------------------
        // 🔥 PROFIL VOIE (N / HO)
        // -------------------------------------------------------------------
        if (message.indexOf("set_profile") >= 0)
        {
            uint8_t profile = doc1["value"] | 0;

            Serial.printf("Changement profil voie demandé : %u\n", profile);

            // Sauvegarde
            DiscoveryMaster_Settings::track_profile = profile;
            DiscoveryMaster_Settings::writeFile();

            // Envoi CAN vers tous les SA
            _can->sendTrackProfile(profile);

            // Mise à jour immédiate de l’UI
            pushStatus();
        }

        // -------------------------------------------------------------------
        // 🟩 CLEAR STOP GLOBAL (0x202)
        // -------------------------------------------------------------------
        if (message.indexOf("clear_stop") >= 0)
        {
            CANMessage msg;
            msg.id = PROTOCOLCAN_ID_CLEAR_STOP; // 0x202
            msg.ext = false;
            msg.len = 0;

            _can->sendMessage(msg);
            Serial.println("[WEB] CLEAR STOP envoyé !");
        }

        // -------------------------------------------------------------------
        // 🟥 STOP GLOBAL (0x201)
        // -------------------------------------------------------------------
        else if (message.indexOf("stop") >= 0)
        {
            CANMessage msg;
            msg.id = PROTOCOLCAN_ID_STOP; // 0x201
            msg.ext = false;
            msg.len = 0;

            _can->sendMessage(msg);
            Serial.println("[WEB] STOP global envoyé !");
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
                            Serial.printf("Not found: %s!\r\n", request->url().c_str());
                            request->send(404); });
}
