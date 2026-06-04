#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "DiscoveryMaster_CanService.h"
#include "DiscoveryMaster_Settings.h"
#include "DiscoveryMaster_SatManager.h"
#include "DiscoveryMaster_Config.h"

class DiscoveryMaster_WebHandler
{
private:
    AsyncWebServer *_server;
    AsyncWebSocket *_ws;
    DiscoveryMaster_CanService *_can;

    void _WsEvent(AsyncWebSocket *server,
                  AsyncWebSocketClient *client,
                  AwsEventType type,
                  void *arg,
                  uint8_t *data,
                  size_t len);

public:
    DiscoveryMaster_WebHandler(DiscoveryMaster_CanService *canService);

    void init(uint16_t webPort);
    void loop();

    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
    void notifyClients();

    // --- AJOUT POUR LE PUSH WEBSOCKET ---
    void pushStatus();

    void route();
};
