#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include "../communication/CommandHandler.h"

class BleManager {
public:
    explicit BleManager(CommandHandler& handler);

    void begin();
    void loop();

private:
    CommandHandler& commandHandler;

    NimBLEServer* server = nullptr;
    NimBLECharacteristic* txCharacteristic = nullptr;

    bool initialized = false;
};