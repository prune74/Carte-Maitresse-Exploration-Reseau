#pragma once

#include "Arduino.h"
#include <ArduinoJson.h>
#include "DiscoveryMaster_Config.h"
#include "DiscoveryMaster_Settings.h"
#include "DiscoveryMaster_CanService.h"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <SPI.h>

class DiscoveryMaster_WebHandler
{
protected:
    AsyncWebServer *_server;
    AsyncWebSocket *_ws;
    DiscoveryMaster_CanService *_can;
    void _WsEvent(AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType, void *, uint8_t *, size_t);

public:
    DiscoveryMaster_WebHandler(DiscoveryMaster_CanService *canService);
    void init(uint16_t webPort);
    void loop();
    void handleWebSocketMessage(void *, uint8_t *, size_t);
    void notifyClients();
    void route();
};
