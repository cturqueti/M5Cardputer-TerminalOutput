#ifndef MYMQTT_H
#define MYMQTT_H
#include <AsyncMqttClient.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#include "M5Cardputer.h"

class MyMqtt {
   public:
    MyMqtt(MDNSResponder* _mdns, M5Canvas* _canvas);
    void startMqtt(const char* hostName, const char* mqttServiceName, const char* mqttUser, const char* mqttPass);
    void stopMqtt();

   private:
    void connectToMqtt(const char* mqttUser, const char* mqttPass);

    AsyncMqttClient mqttClient;
    M5Canvas* canvas;
    MDNSResponder* mdns;
    IPAddress mqttHost;
    uint16_t mqttPort;
};
#endif