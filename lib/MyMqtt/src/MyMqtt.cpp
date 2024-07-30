#include "MyMqtt.h"

MyMqtt::MyMqtt(MDNSResponder* _mdns, M5Canvas* _canvas) : mdns(_mdns), canvas(_canvas) {}

void MyMqtt::startMqtt(const char* hostName, const char* mqttServiceName, const char* mqttUser, const char* mqttPass) {
    if (mdns->begin(hostName)) {  // Certifique-se de que o mDNS foi inicializado corretamente
        int n = mdns->queryService(mqttServiceName, "_tcp");
        if (n > 0) {
            mqttHost = mdns->IP(0);  // Pega o IP do primeiro serviço encontrado
            mqttPort = mdns->port(0);
            canvas->print("MQTT server IP resolved: ");
            canvas->println(mqttHost.toString().c_str());
            canvas->print("MQTT server port resolved: ");
            canvas->println(mqttPort);
            delay(1000);
            connectToMqtt(mqttUser, mqttPass);
        } else {
            canvas->println("MQTT server not found via mDNS.");
        }
    } else {
        canvas->println("mDNS initialization failed.");
    }
}

void MyMqtt::connectToMqtt(const char* mqttUser, const char* mqttPass) {
    canvas->println("Connecting to MQTT...");
    mqttClient.setCredentials(mqttUser, mqttPass);
    mqttClient.setServer(mqttHost, mqttPort);
    mqttClient.connect();
}

void MyMqtt::stopMqtt() {
}
