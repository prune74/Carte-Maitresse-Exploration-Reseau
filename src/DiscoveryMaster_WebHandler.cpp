/*
DiscoveryMaster_WebHandler.cpp / .h

🎯 Rôle
Serveur Web et interface de communication du module SAMain.
Ce module gère :
- l’hébergement des pages HTML/CSS/JS
- l’API Discovery via WebSocket
- la réception des commandes envoyées depuis l’interface Web
- la transmission des ordres au bus CAN Discovery via DiscoveryMaster_CanService

Il constitue l’interface utilisateur du système Discovery 2026.

📌 Fonctionnement
- init(port) :
    • crée un serveur web AsyncWebServer
    • crée un WebSocket /ws
    • attache un callback d’événements WebSocket
    • configure les routes HTTP (fichiers SPIFFS)
    • démarre le serveur

- loop() :
    • nettoie les clients WebSocket inactifs

- _WsEvent() :
    • gère les événements WebSocket :
        - connexion / déconnexion
        - réception de données JSON
    • décode les commandes envoyées par l’interface Web :
        - wifi_on        → active/désactive le WiFi
        - discovery_on   → active/désactive le réseau Discovery
        - save           → demande de sauvegarde globale
        - restartEsp     → demande de redémarrage des satellites
    • transmet les commandes au bus CAN via DiscoveryMaster_CanService

- handleWebSocketMessage() :
    • gestion simplifiée de messages texte (ex : "toggle")

- route() :
    • définit les routes HTTP pour servir :
        - index.html
        - w3.css
        - style.css
        - script.js
        - settings.json
        - favicon.png
    • gère les erreurs 404

📌 Particularités
- Utilise AsyncWebServer et AsyncWebSocket pour un fonctionnement non bloquant.
- Le WebSocket permet un échange bidirectionnel en temps réel avec l’interface Web.
- Le module agit comme un pont entre l’utilisateur et le réseau Discovery.
- Les commandes reçues sont immédiatement propagées au bus CAN via _can->sendXXX().
- L’interface Web peut piloter l’ensemble du réseau Discovery 2026.

🔗 Dépendances
- DiscoveryMaster_CanService (envoi des commandes Discovery)
- DiscoveryMaster_Settings (mise à jour des paramètres)
- SPIFFS (fichiers HTML/CSS/JS)
- ArduinoJson (décodage des messages WebSocket)
*/

#include "DiscoveryMaster_WebHandler.h"

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
        Serial.printf("WebSocket client #%u connected from %s\n",
                      client->id(),
                      client->remoteIP().toString().c_str());
        break;

    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;

    case WS_EVT_DATA:
    {
        StaticJsonDocument<1024> doc1;
        DeserializationError error = deserializeJson(doc1, data);

        if (error)
        {
#ifdef DEBUG
            debug.println("Parsing failed");
#endif
            return;
        }

        String message = (char *)data;

        // --- WIFI ON/OFF ---
        if (message.indexOf("wifi_on") >= 0)
        {
            DiscoveryMaster_Settings::WIFI_ON = doc1["wifi_on"].as<bool>();
            _can->sendWifiOnOff(DiscoveryMaster_Settings::WIFI_ON);
            DiscoveryMaster_Settings::writeFile();  // Persist setting

#ifdef DEBUG
            debug.printf(DiscoveryMaster_Settings::WIFI_ON ? "Wifi : on\n" : "Wifi : off\n");
#endif
        }

        // --- DISCOVERY ON/OFF ---
        if (message.indexOf("discovery_on") >= 0)
        {
            DiscoveryMaster_Settings::DISCOVERY_ON = doc1["discovery_on"].as<bool>();
            _can->sendDiscoveryOnOff(DiscoveryMaster_Settings::DISCOVERY_ON);
            DiscoveryMaster_Settings::writeFile();  // Persist setting

#ifdef DEBUG
            debug.printf(DiscoveryMaster_Settings::DISCOVERY_ON ? "Discovery : on\n" : "Discovery : off\n");
#endif
        }

        // --- SAVE ---
        if (message.indexOf("save") >= 0)
        {
#ifdef DEBUG
            debug.println("save all");
#endif
            _can->sendSaveAll();
        }

        // --- RESTART ---
        if (message.indexOf("restartEsp") >= 0)
        {
#ifdef DEBUG
            debug.println("restartEsp");
#endif
            _can->sendRestartAll();
        }
    }
    break;
    }
}

void DiscoveryMaster_WebHandler::notifyClients()
{
    _ws->textAll(String("ok"));
}

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
