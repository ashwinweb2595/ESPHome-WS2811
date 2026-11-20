#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include "../config/DeviceConfig.h"

class SettingsStore {
public:
    bool begin();
    DeviceSettings load();
    bool save(const DeviceSettings& settings);
    void close();

private:
    Preferences preferences;
    bool initialized = false;
};