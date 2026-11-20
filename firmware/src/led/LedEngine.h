
#pragma once

#include <Arduino.h>
#include <FastLED.h>
#include <vector>

#include "../config/DeviceConfig.h"

class LedEngine {
public:
    // Initialize with the default 66 addressable pixels.
    void begin();

    // Initialize with a configurable number of addressable pixels.
    bool begin(uint32_t totalLeds);

    // Legacy compatibility during migration.
    void applySettings(const DeviceSettings& settings);
    DeviceSettings getSettings() const;

    // New dynamic configuration API.
    bool applyDynamicSettings(const DynamicDeviceSettings& settings);
    DynamicDeviceSettings getDynamicSettings() const;

    // Master brightness.
    void setMasterBrightness(uint8_t brightness);

    // Legacy segment operations by array index.
    bool setSegmentColor(uint8_t segment, uint32_t color);
    bool setSegmentBrightness(uint8_t segment, uint8_t brightness);
    bool setSegmentEnabled(uint8_t segment, bool enabled);

    // Dynamic segment operations by unique ID.
    bool setSegmentColorById(uint32_t id, uint32_t color);
    bool setSegmentBrightnessById(uint32_t id, uint8_t brightness);
    bool setSegmentEnabledById(uint32_t id, bool enabled);

    // Dynamic segment management.
    bool addSegment(const DynamicSegmentSettings& segment);
    bool updateSegment(const DynamicSegmentSettings& segment);
    bool removeSegment(uint32_t id);

    // Individual addressable pixel control.
    bool setLedColor(uint32_t index, uint32_t color);

    void clear();
    void show();

    uint32_t getPixelCount() const {
        return pixelCount;
    }

    bool isInitialized() const {
        return initialized;
    }

private:
    DynamicDeviceSettings currentSettings{};

    std::vector<CRGB> leds;
    std::vector<uint32_t> ledOverrides;
    std::vector<uint8_t> overrideEnabled;

    uint32_t pixelCount = 0;
    bool initialized = false;

    DynamicSegmentSettings* findSegment(uint32_t id);
    bool validateSettings(const DynamicDeviceSettings& settings) const;

    void render();
};
