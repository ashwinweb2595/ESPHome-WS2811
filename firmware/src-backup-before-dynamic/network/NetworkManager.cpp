#include "NetworkManager.h"

#include <WiFi.h>
#include <ArduinoJson.h>
#include "secrets.h"

NetworkManager::NetworkManager(CommandHandler& handler)
    : commandHandler(handler), server(80) {
}

void NetworkManager::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.setHostname("smart-led-controller");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.println("Connecting to Wi-Fi...");
}

void NetworkManager::loop() {
    if (WiFi.status() == WL_CONNECTED) {
        if (!serverStarted) {
            startServer();
        }

        server.handleClient();
        return;
    }

    if (serverStarted) {
        serverStarted = false;
        Serial.println("Wi-Fi disconnected. HTTP server unavailable.");
    }

    const unsigned long now = millis();

    if (now - lastReconnectAttempt >= 10000) {
        lastReconnectAttempt = now;
        Serial.println("Attempting Wi-Fi reconnection...");
        WiFi.reconnect();
    }
}

void NetworkManager::startServer() {
    server.on("/api/v1/status", HTTP_GET, [this]() {
        handleStatus();
    });

    server.on("/api/v1/command", HTTP_POST, [this]() {
        handleCommand();
    });

    server.onNotFound([this]() {
        server.send(404, "application/json",
                    "{\"ok\":false,\"message\":\"Endpoint not found\"}");
    });

    server.begin();
    serverStarted = true;

    Serial.println("HTTP server started.");
    Serial.print("ESP32 IP address: ");
    Serial.println(WiFi.localIP());
}

void NetworkManager::handleStatus() {
    JsonDocument response;

    response["ok"] = true;
    response["device"] = "smart-led-controller";
    response["protocol"] = 1;
    response["wifi_connected"] = true;
    response["ip"] = WiFi.localIP().toString();
    response["rssi"] = WiFi.RSSI();

    String output;
    serializeJson(response, output);

    server.send(200, "application/json", output);
}

void NetworkManager::handleCommand() {
    String body = server.arg("plain");

    if (body.isEmpty()) {
        server.send(400, "application/json",
                    "{\"ok\":false,\"message\":\"Empty request body\"}");
        return;
    }

    String response = commandHandler.handle(body);

    server.send(200, "application/json", response);
}