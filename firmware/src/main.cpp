
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

    // 1. Initialize persistent storage.
    if (!settingsStore.begin()) {
        Serial.println("Settings storage unavailable.");
    }

    // 2. Load dynamic settings before initializing FastLED.
    DynamicDeviceSettings settings =
        settingsStore.loadDynamic(66);

    Serial.printf(
        "Configured addressable pixels: %lu\n",
        static_cast<unsigned long>(settings.totalLeds)
    );

    Serial.printf(
        "Configured segments: %u\n",
        static_cast<unsigned int>(settings.segments.size())
    );

    // 3. Initialize the LED engine with the saved pixel count.
    if (!ledEngine.begin(settings.totalLeds)) {
        Serial.println("ERROR: LED engine initialization failed.");
        return;
    }

    // 4. Apply saved dynamic configuration.
    if (!ledEngine.applyDynamicSettings(settings)) {
        Serial.println("ERROR: Unable to apply saved configuration.");
        return;
    }

    Serial.println("Dynamic LED configuration restored.");

    // 5. Start communication services.
    Serial.println("JSON command handler initialized.");

    networkManager.begin();
    bleManager.begin();

    Serial.println("Firmware initialization complete.");
}

void loop() {
    networkManager.loop();
    bleManager.loop();

    delay(2);
}
