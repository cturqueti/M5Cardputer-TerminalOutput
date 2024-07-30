// Instalar o avahi no servidor
//
//
//

#include <AsyncMqttClient.h>
#include <ConfigWifi.h>
#include <ESPmDNS.h>
#include <M5Cardputer.h>
#include <Preferences.h>  // Biblioteca para NVS
#include <WiFi.h>

const char* mqtt_user = "sideout";
const char* mqtt_passwd = "sidetouch";
const char* mqttServiceName = "_mqtt";
const char* protocol = "_tcp";

const char* hostName = "modulo1";
const char* apSSID = "ESP32-Setup";

#define MAX_LINES 100
#define LINE_HEIGHT 16

String lineBuffer[MAX_LINES];
int scrollOffset = 0;
int maxLines = MAX_LINES;

String module = String(hostName);

struct TopicParts {
    std::string part1;
    std::string part2;
    bool success;
};

AsyncMqttClient mqttClient(hostName);
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;
TimerHandle_t displayRefresh;
IPAddress mqttIP;
uint16_t mqttPort;
Preferences preferences;
MDNSResponder mdns;
M5Canvas canvas(&M5Cardputer.Display);
ConfigWifi configWifi(&preferences, &canvas, &mdns);

void connectToWifi() {
    configWifi.connectToWifi();
}

void connectToMqtt() {
    canvas.println("Connecting to MQTT...");
    mqttClient.setCredentials(mqtt_user, mqtt_passwd);
    mqttClient.setServer(mqttServiceName, protocol);
    mqttClient.connect();
}

void displayRefreshFunction(TimerHandle_t xTimer) {
    canvas.pushSprite(0, 0);
}

void WiFiEvent(WiFiEvent_t event) {
    canvas.printf("[WiFi-event] event: %d\n", event);
    switch (event) {
        case SYSTEM_EVENT_STA_GOT_IP:
            canvas.println("WiFi connected");
            canvas.println("IP address: ");
            canvas.println(WiFi.localIP().toString().c_str());
            connectToMqtt();
            break;

        case SYSTEM_EVENT_STA_DISCONNECTED:
            canvas.println("WiFi lost connection");
            xTimerStop(mqttReconnectTimer, 0);  // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
            xTimerStart(wifiReconnectTimer, 0);
            break;
    }
}

void onMqttConnect(bool sessionPresent) {
    canvas.println("Connected to MQTT.");
    canvas.print("Session present: ");
    canvas.println(sessionPresent ? "true" : "false");

    // Subscribing to topics, resubscribe if sessionPresent is false
    if (!sessionPresent) {
        uint16_t packetIdSub = mqttClient.subscribe((module + "/#").c_str(), 2);
        canvas.print("Subscribing to ");
        canvas.print(module.c_str());
        canvas.println("/# at QoS 2, packetId: ");
        canvas.println(String(packetIdSub).c_str());
        packetIdSub = mqttClient.subscribe("broadcast/#", 2);
        canvas.println("Subscribing to broadcast/# at QoS 2, packetId: ");
        canvas.println(String(packetIdSub).c_str());
    }
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    canvas.println("Disconnected from MQTT.");

    if (WiFi.isConnected()) {
        xTimerStart(mqttReconnectTimer, 0);
    }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
    canvas.println("Subscribe acknowledged.");
    canvas.print("  packetId: ");
    canvas.println(String(packetId).c_str());
    canvas.print("  qos: ");
    canvas.println(String(qos).c_str());
}

void onMqttUnsubscribe(uint16_t packetId) {
    canvas.println("Unsubscribe acknowledged.");
    canvas.print("  packetId: ");
    canvas.println(String(packetId).c_str());
}

String removePrefix(const String& topic, const String& prefix) {
    if (topic.startsWith(prefix)) {
        return topic.substring(prefix.length());
    }
    return topic;
}

TopicParts splitTopic(const char* topic) {
    char topicCopy[128];
    strncpy(topicCopy, topic, sizeof(topicCopy) - 1);
    topicCopy[sizeof(topicCopy) - 1] = '\0';  // Certifique-se de que a string é terminada com nulo

    // Use strtok para separar a string
    char* part1 = strtok(topicCopy, "/");
    char* part2 = strtok(NULL, "/");

    TopicParts result;
    if (part1 != NULL && part2 != NULL) {
        result.part1 = std::string(part1);
        result.part2 = std::string(part2);
        result.success = true;
    } else {
        result.success = false;
    }
    return result;
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    TopicParts parts = splitTopic(topic);
    canvas.println("");

    if (parts.success) {
        canvas.println("Publish received.");
        canvas.print("  topic: ");
        canvas.println(parts.part2.c_str());
        canvas.print("  qos: ");
        canvas.print(String(properties.qos).c_str());
        // canvas.print("  dup: ");
        // canvas.println(properties.dup ? "true" : "false");
        // canvas.print("  retain: ");
        // canvas.println(properties.retain ? "true" : "false");
        canvas.print("  len: ");
        canvas.print(String(len).c_str());
        // canvas.print("  index: ");
        // canvas.println(String(index).c_str());
        canvas.print("  total: ");
        canvas.println(String(total).c_str());

        String payloadStr = String(payload).substring(0, len);  // Ensure correct length
        payloadStr.trim();
        canvas.print("  payload: ");
        canvas.println(payloadStr.c_str());

        if (parts.part1 == module.c_str() || parts.part1 == "broadcast") {
            if (parts.part2 == "read" && payloadStr == "ping") {
                canvas.println("terei que responder");
                mqttClient.publish(String(module + "/ip").c_str(), 1, true, WiFi.localIP().toString().c_str());
                // Adicione aqui a lógica para responder ao comando
            }
        }
    }
}

void onMqttPublish(uint16_t packetId) {
    canvas.println("Publish acknowledged.");
    canvas.print("  packetId: ");
    canvas.println(String(packetId).c_str());
}

void addLineToBuffer(const String& text) {
    // Move as linhas para cima para fazer espaço para a nova linha
    for (int i = MAX_LINES - 1; i > 0; --i) {
        lineBuffer[i] = lineBuffer[i - 1];
    }
    // Adiciona o novo texto à primeira linha
    lineBuffer[0] = text;
}

void setup(void) {
    auto cfg = M5.config();
    M5Cardputer.begin(cfg);

    M5Cardputer.Display.setRotation(1);

    if (M5Cardputer.Display.isEPD()) {
        M5Cardputer.Display.setEpdMode(epd_mode_t::epd_fastest);
        M5Cardputer.Display.invertDisplay(true);
        M5Cardputer.Display.clear(TFT_BLACK);
    }
    if (M5Cardputer.Display.width() < M5Cardputer.Display.height()) {
        M5Cardputer.Display.setRotation(M5Cardputer.Display.getRotation() ^ 1);
    }

    canvas.setColorDepth(1);  // mono color
    canvas.createSprite(M5Cardputer.Display.width(), M5Cardputer.Display.height());
    canvas.setTextFont(&fonts::DejaVu9);
    // canvas.setTextSize((float)canvas.width() / 160);
    canvas.setTextScroll(true);
    // canvas.setTextColor(TFT_GREEN);
    //   canvas.setTextFont(&fonts::FreeSans9pt7b);

    displayRefresh = xTimerCreate("diplayRefreshTimer", pdMS_TO_TICKS(100), pdTRUE, (void*)0, displayRefreshFunction);
    xTimerStart(displayRefresh, 0);

    canvas.println("Configurar Wifi, pressione: (/)");

    M5Cardputer.update();
    if (M5Cardputer.Keyboard.isKeyPressed('/')) {
        // canvas.println("------>passou por aqui<-------");
        // delay(2000);
        configWifi.begin();

        while (true) {
            configWifi.handleClient();
        }
    }

    mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
    wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

    WiFi.onEvent(WiFiEvent);

    mqttClient.onConnect(onMqttConnect);
    mqttClient.onDisconnect(onMqttDisconnect);
    mqttClient.onSubscribe(onMqttSubscribe);
    mqttClient.onUnsubscribe(onMqttUnsubscribe);
    mqttClient.onMessage(onMqttMessage);
    mqttClient.onPublish(onMqttPublish);
    // mqttClient.setServer(MQTT_HOST, MQTT_PORT);

    connectToWifi();
}

void loop(void) {
    // ArduinoOTA.handle();
    M5Cardputer.update();

    // canvas.pushSprite(0, 0);
}
