#include "SettingsStore.h"

bool SettingsStore::begin() {
    initialized = preferences.begin("smart-led", false);
    return initialized;
}

DeviceSettings SettingsStore::load() {
    DeviceSettings settings = defaultDeviceSettings();

    if (!initialized) return settings;

    const size_t storedSize = preferences.getBytesLength("settings");

    if (storedSize != sizeof(DeviceSettings)) {
        save(settings);
        return settings;
    }

    if (preferences.getBytes("settings", &settings,
                             sizeof(DeviceSettings)) != sizeof(DeviceSettings)) {
        settings = defaultDeviceSettings();
        save(settings);
        return settings;
    }

    if (settings.version != DeviceConfig::SETTINGS_VERSION ||
        settings.masterBrightness > 100) {
        settings = defaultDeviceSettings();
        save(settings);
        return settings;
    }

    for (uint8_t i = 0; i < DeviceConfig::SEGMENT_COUNT; i++) {
        auto& segment = settings.segments[i];

        if (segment.start >= DeviceConfig::LED_COUNT ||
            segment.end >= DeviceConfig::LED_COUNT ||
            segment.start > segment.end ||
            segment.brightness > 100) {
            settings = defaultDeviceSettings();
            save(settings);
            return settings;
        }
    }

    return settings;
}

bool SettingsStore::save(const DeviceSettings& settings) {
    if (!initialized) return false;

    return preferences.putBytes(
        "settings", &settings, sizeof(DeviceSettings)
    ) == sizeof(DeviceSettings);
}

void SettingsStore::close() {
    if (initialized) {
        preferences.end();
        initialized = false;
    }
}