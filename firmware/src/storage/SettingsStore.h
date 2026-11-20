
#pragma once

#include <Arduino.h>
#include <Preferences.h>

#include "../config/DeviceConfig.h"

class SettingsStore {
public:
    bool begin();

    // Legacy storage methods.
    DeviceSettings load();
    bool save(const DeviceSettings& settings);

    // New dynamic JSON storage methods.
    DynamicDeviceSettings loadDynamic(
        uint32_t defaultTotalLeds = 66
    );

    bool saveDynamic(
        const DynamicDeviceSettings& settings
    );

    void close();

private:
    Preferences preferences;
    bool initialized = false;
};
