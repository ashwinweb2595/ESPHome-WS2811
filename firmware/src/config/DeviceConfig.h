
#pragma once

#include <Arduino.h>
#include <stdint.h>
#include <vector>

namespace DeviceConfig {
    constexpr uint8_t LED_PIN = 23;

    // Configurable hardware capacity limits.
    constexpr uint32_t MAX_PIXEL_COUNT = 1000;
    constexpr uint16_t MAX_SEGMENTS = 64;

    constexpr uint8_t DEFAULT_BRIGHTNESS = 35;
    constexpr uint16_t SETTINGS_VERSION = 2;
    constexpr char DEVICE_NAME[] = "Smart LED Controller";

    // Legacy constants retained during migration.
    constexpr uint16_t LED_COUNT = 66;
    constexpr uint8_t SEGMENT_COUNT = 6;
}

// New dynamic segment model.
struct DynamicSegmentSettings {
    uint32_t id = 0;
    String name = "";

    // Zero-based addressable pixel position.
    uint32_t start = 0;

    // Number of addressable pixels in this segment.
    uint32_t count = 0;

    uint8_t brightness = 100;
    uint32_t color = 0xFFFFFF;
    bool enabled = false;
};

// New dynamic device configuration.
struct DynamicDeviceSettings {
    uint16_t version = DeviceConfig::SETTINGS_VERSION;

    uint32_t totalLeds = 66;
    uint8_t masterBrightness = DeviceConfig::DEFAULT_BRIGHTNESS;

    std::vector<DynamicSegmentSettings> segments;
};

// Creates a default dynamic configuration.
inline DynamicDeviceSettings defaultDynamicDeviceSettings(
    uint32_t totalLeds = 66
) {
    DynamicDeviceSettings settings;

    settings.totalLeds = totalLeds;

    return settings;
}

// --------------------------------------------------
// Legacy configuration model.
// Retained temporarily for backward-compatible migration.
// --------------------------------------------------

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

    settings.version = 1;
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
