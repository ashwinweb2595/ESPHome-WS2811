#include <Arduino.h>

#include "config/DeviceConfig.h"
#include "led/LedEngine.h"
#include "storage/SettingsStore.h"

LedEngine ledEngine;
SettingsStore settingsStore;

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

    Serial.println("LED engine initialized.");
    Serial.printf("LED sections: %u\n", DeviceConfig::LED_COUNT);
    Serial.printf("Segments: %u\n", DeviceConfig::SEGMENT_COUNT);
    Serial.println("Firmware initialization complete.");
}

void loop() {
    delay(1000);
}