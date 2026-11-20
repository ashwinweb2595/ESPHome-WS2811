#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include "../communication/CommandHandler.h"

class NetworkManager {
public:
    explicit NetworkManager(CommandHandler& handler);

    void begin();
    void loop();

private:
    CommandHandler& commandHandler;
    WebServer server;
    bool serverStarted = false;
    unsigned long lastReconnectAttempt = 0;

    void startServer();
    void handleStatus();
    void handleCommand();
};