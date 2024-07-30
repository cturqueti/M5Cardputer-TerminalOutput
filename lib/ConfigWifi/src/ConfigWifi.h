#ifndef CONFIGWIFI_H
#define CONFIGWIFI_H

#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>
#include <Preferences.h>
#include <SPIFFS.h>
#include <WiFi.h>

#include "M5Cardputer.h"

class ConfigWifi {
   public:
    ConfigWifi(Preferences* _preferences, M5Canvas* _canvas, MDNSResponder* _mdns);  // Nome correto do construtor
    void begin();
    void handleClient();
    void connectToWifi();

   private:
    void startAP();
    void startServer();
    void handleRoot(AsyncWebServerRequest* request);
    void handleConfig(AsyncWebServerRequest* request);
    void setupMDNS();  // Configuração do mDNS

    Preferences* preferences;  // Ponteiro para a instância de Preferences
    M5Canvas* canvas;
    MDNSResponder* mdns;

    const char* ssid = "ConfigWifi";
    const char* password = "";
    AsyncWebServer server;
    DNSServer dnsServer;
};

#endif  // CONFIGWIFI_H
