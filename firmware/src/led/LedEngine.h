#pragma once

#include <Arduino.h>
#include "../config/DeviceConfig.h"

class LedEngine {
public:
    void begin();
    void applySettings(const DeviceSettings& settings);
    DeviceSettings getSettings() const {
        return currentSettings;
    }

    void setMasterBrightness(uint8_t brightness);
    bool setSegmentColor(uint8_t segment, uint32_t color);
    bool setSegmentBrightness(uint8_t segment, uint8_t brightness);
    bool setSegmentEnabled(uint8_t segment, bool enabled);
    bool setLedColor(uint8_t index, uint32_t color);
    void clear();
    void show();

private:
    DeviceSettings currentSettings{};
    uint32_t ledOverrides[DeviceConfig::LED_COUNT]{};
    bool overrideEnabled[DeviceConfig::LED_COUNT]{};

    void render();
};