#pragma once

#include <Arduino.h>
#include <stdint.h>

namespace DeviceConfig {
    constexpr uint8_t LED_PIN = 23;
    constexpr uint16_t LED_COUNT = 66;
    constexpr uint8_t SEGMENT_COUNT = 6;
    constexpr uint8_t DEFAULT_BRIGHTNESS = 35;
    constexpr uint16_t SETTINGS_VERSION = 1;
    constexpr char DEVICE_NAME[] = "Smart LED Controller";
}

struct LedSegmentSettings {
    uint8_t start;
    uint8_t end;
    uint8_t brightness;
    uint32_t color;
    uint8_t enabled;
};

struct DeviceSettings {
    uint16_t version;
    uint8_t masterBrightness;
    LedSegmentSettings segments[DeviceConfig::SEGMENT_COUNT];
};

inline DeviceSettings defaultDeviceSettings() {
    DeviceSettings settings{};
    settings.version = DeviceConfig::SETTINGS_VERSION;
    settings.masterBrightness = DeviceConfig::DEFAULT_BRIGHTNESS;

    for (uint8_t i = 0; i < DeviceConfig::SEGMENT_COUNT; i++) {
        settings.segments[i].start = i * 11;
        settings.segments[i].end = (i * 11) + 10;
        settings.segments[i].brightness = 100;
        settings.segments[i].color = 0xFFFFFF;
        settings.segments[i].enabled = false;
    }

    return settings;
}