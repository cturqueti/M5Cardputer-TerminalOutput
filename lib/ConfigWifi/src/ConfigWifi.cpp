#include "ConfigWifi.h"

// Construtor com o nome correto
ConfigWifi::ConfigWifi(Preferences *_preferences, M5Canvas *_canvas, MDNSResponder *_mdns) : preferences(_preferences), canvas(_canvas), mdns(_mdns), server(80) {}

void ConfigWifi::begin() {
    // Inicializa o SPIFFS
    if (!SPIFFS.begin(true)) {
        canvas->println("Falha ao montar o sistema de arquivos SPIFFS");
        return;
    }
    startAP();
    setupMDNS();  // Configurar mDNS
    startServer();
}

void ConfigWifi::startAP() {
    WiFi.softAP(ssid);
    IPAddress IP = WiFi.softAPIP();
    canvas->println("Access Point Started");
    canvas->print("IP Address: ");
    canvas->println(IP);
    // Configuração do servidor DNS para redirecionar todas as solicitações para o IP do ESP32
    dnsServer.start(53, "*", IP);
}

void ConfigWifi::setupMDNS() {
    if (!mdns->begin("configwifi")) {  // Nome de host para mDNS
        canvas->println("Falha ao iniciar o mDNS");
    } else {
        canvas->println("mDNS iniciado. Acesse http://configwifi.local para configurar.");
    }
}

void ConfigWifi::startServer() {
    server.on("/", HTTP_GET, std::bind(&ConfigWifi::handleRoot, this, std::placeholders::_1));
    server.on("/config", HTTP_POST, std::bind(&ConfigWifi::handleConfig, this, std::placeholders::_1));
    server.begin();
    canvas->println("HTTP server started");
}

void ConfigWifi::handleRoot(AsyncWebServerRequest *request) {
    // Carrega a página HTML do SPIFFS
    String htmlPath = "/config.html";  // Caminho para seu arquivo HTML no SPIFFS

    // Lê o conteúdo do arquivo HTML
    File file = SPIFFS.open(htmlPath, "r");
    if (!file) {
        request->send(404, "text/plain", "Página não encontrada");
        canvas->println("Página não encontrada");
        return;
    }

    // Envia o conteúdo do arquivo HTML como resposta
    String html = file.readString();
    file.close();

    request->send(200, "text/html", html);
    canvas->println("Página HTML enviada");
}

void ConfigWifi::handleConfig(AsyncWebServerRequest *request) {
    String ssid = request->getParam("ssid", true)->value();
    String password = request->getParam("password", true)->value();

    // Salve o SSID e senha recebidos em uma memória não volátil
    preferences->begin("wifi", false);
    preferences->putString("ssid", ssid);
    preferences->putString("password", password);
    preferences->end();

    String htmlPath = "/sucess.html";
    // Lê o conteúdo do arquivo HTML
    File file = SPIFFS.open(htmlPath, "r");
    if (!file) {
        request->send(404, "text/plain", "Página não encontrada");
        canvas->println("Página não encontrada");
        return;
    }

    // Envia o conteúdo do arquivo HTML como resposta
    String html = file.readString();
    file.close();

    request->send(200, "text/html", html);

    // Desconecte o AP e reinicie o ESP para conectar-se à nova rede
    WiFi.softAPdisconnect(true);
    delay(1000);
    mdns->end();
    ESP.restart();
}

void ConfigWifi::connectToWifi() {
    preferences->begin("wifi", true);
    String ssid = preferences->getString("ssid", "");
    String password = preferences->getString("password", "");
    preferences->end();

    if (ssid.length() > 0 && password.length() > 0) {
        canvas->println("Connecting to Wi-Fi...");
        WiFi.begin(ssid.c_str(), password.c_str());

        int attempts = 0;
        const int maxAttempts = 10;  // Número máximo de tentativas de conexão

        while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
            delay(500);
            canvas->print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            canvas->println("Connected to Wi-Fi!");
            canvas->print("IP Address: ");
            canvas->println(WiFi.localIP());
            // Aqui você pode adicionar mais código para continuar a configuração
        } else {
            canvas->println("Failed to connect to Wi-Fi.");
            // O dispositivo pode iniciar o AP de configuração novamente se necessário
            startAP();
        }
    } else {
        canvas->println("No Wi-Fi credentials found. Starting AP...");
        startAP();
    }
}

void ConfigWifi::handleClient() {
    dnsServer.processNextRequest();
}