#pragma once

#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

#include "CanMsg.h"
#include "ExplorationReseau_Maitre_CanService.h"
#include "ExplorationReseau_Maitre_SatManager.h"

/*
 * 🎯 Rôle
 * Interface Web + WebSocket de la Carte Maîtresse ERM.
 *
 * Fournit :
 *   • serveur HTTP
 *   • WebSocket bidirectionnel
 *   • réception des commandes UI
 *   • diffusion de l’état complet du système
 */

class ERM_WebHandler
{
public:
    ERM_WebHandler(ERM_CanService *canService);

    void init(uint16_t webPort);
    void loop();

    void pushStatus();
    void notifyClients();

private:
    void ERM_route();
    void ERM_wsEvent(AsyncWebSocket *server,
                     AsyncWebSocketClient *client,
                     AwsEventType type,
                     void *arg,
                     uint8_t *data,
                     size_t len);

    void ERM_handleSimpleMessage(void *arg, uint8_t *data, size_t len);

private:
    AsyncWebServer  *_server;
    AsyncWebSocket  *_ws;
    ERM_CanService  *_can;
};
