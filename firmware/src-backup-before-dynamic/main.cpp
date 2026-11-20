#include <Arduino.h>

#include "config/DeviceConfig.h"
#include "led/LedEngine.h"
#include "storage/SettingsStore.h"
#include "communication/CommandHandler.h"
#include "network/NetworkManager.h"
#include "ble/BleManager.h"

LedEngine ledEngine;
SettingsStore settingsStore;
CommandHandler commandHandler(ledEngine, settingsStore);

NetworkManager networkManager(commandHandler);
BleManager bleManager(commandHandler);

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println();
    Serial.println("Smart LED Controller starting...");

    ledEngine.begin();

    if (settingsStore.begin()) {
        DeviceSettings settings = settingsStore.load();
        ledEngine.applySettings(settings);
        Serial.println("Settings restored from ESP32 storage.");
    } else {
        Serial.println("Settings storage unavailable. Using defaults.");
    }

    Serial.println("JSON command handler initialized.");

    Serial.printf("LED sections: %u\n", DeviceConfig::LED_COUNT);
    Serial.printf("Segments: %u\n", DeviceConfig::SEGMENT_COUNT);

    networkManager.begin();
    bleManager.begin();

    Serial.println("Firmware initialization complete.");
}

void loop() {
    networkManager.loop();
    bleManager.loop();

    delay(2);
}