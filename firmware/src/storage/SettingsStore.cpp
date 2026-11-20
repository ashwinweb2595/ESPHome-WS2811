
#include "SettingsStore.h"

#include <ArduinoJson.h>

// Separate keys ensure the old binary settings are preserved.
static constexpr const char* LEGACY_SETTINGS_KEY = "settings";
static constexpr const char* DYNAMIC_SETTINGS_KEY = "settings_v2";

bool SettingsStore::begin() {
    initialized = preferences.begin("smart-led", false);

    return initialized;
}

// --------------------------------------------------
// LEGACY STORAGE
// --------------------------------------------------

DeviceSettings SettingsStore::load() {
    DeviceSettings settings = defaultDeviceSettings();

    if (!initialized) {
        return settings;
    }

    const size_t storedSize =
        preferences.getBytesLength(LEGACY_SETTINGS_KEY);

    if (storedSize != sizeof(DeviceSettings)) {
        return settings;
    }

    if (preferences.getBytes(
            LEGACY_SETTINGS_KEY,
            &settings,
            sizeof(DeviceSettings)
        ) != sizeof(DeviceSettings)) {
        return defaultDeviceSettings();
    }

    // Legacy DeviceSettings uses version 1.
    if (settings.version != 1 ||
        settings.masterBrightness > 100) {
        return defaultDeviceSettings();
    }

    for (uint8_t i = 0; i < DeviceConfig::SEGMENT_COUNT; i++) {
        const auto& segment = settings.segments[i];

        if (segment.start >= DeviceConfig::LED_COUNT ||
            segment.end >= DeviceConfig::LED_COUNT ||
            segment.start > segment.end ||
            segment.brightness > 100) {
            return defaultDeviceSettings();
        }
    }

    return settings;
}

bool SettingsStore::save(const DeviceSettings& settings) {
    if (!initialized) {
        return false;
    }

    return preferences.putBytes(
        LEGACY_SETTINGS_KEY,
        &settings,
        sizeof(DeviceSettings)
    ) == sizeof(DeviceSettings);
}

// --------------------------------------------------
// DYNAMIC SETTINGS LOAD
// --------------------------------------------------

DynamicDeviceSettings SettingsStore::loadDynamic(
    uint32_t defaultTotalLeds
) {
    DynamicDeviceSettings defaults =
        defaultDynamicDeviceSettings(defaultTotalLeds);

    if (!initialized) {
        return defaults;
    }

    String json = preferences.getString(
        DYNAMIC_SETTINGS_KEY,
        ""
    );

    if (json.isEmpty()) {
            Serial.println("No dynamic settings found. Checking legacy settings...");

            DeviceSettings legacy = load();

            DynamicDeviceSettings migrated =
                defaultDynamicDeviceSettings(defaultTotalLeds);

            migrated.masterBrightness =
                constrain(legacy.masterBrightness, 0, 100);

            for (uint8_t i = 0; i < DeviceConfig::SEGMENT_COUNT; i++) {
                const auto& oldSegment = legacy.segments[i];

                if (oldSegment.start > oldSegment.end ||
                    oldSegment.start >= defaultTotalLeds ||
                    oldSegment.end >= defaultTotalLeds) {
                    continue;
                }

                DynamicSegmentSettings segment;

                segment.id = i + 1;
                segment.name = "Segment " + String(i + 1);
                segment.start = oldSegment.start;
                segment.count =
                    static_cast<uint32_t>(oldSegment.end) -
                    oldSegment.start + 1;

                segment.brightness = oldSegment.brightness;
                segment.color = oldSegment.color & 0xFFFFFF;
                segment.enabled = oldSegment.enabled != 0;

                migrated.segments.push_back(segment);
            }

            if (saveDynamic(migrated)) {
                Serial.println("Legacy settings migrated successfully.");
            } else {
                Serial.println("Legacy migration could not be saved.");
            }

            return migrated;
        }

    JsonDocument document;

    DeserializationError error =
        deserializeJson(document, json);

    if (error) {
        Serial.print("Dynamic settings JSON error: ");
        Serial.println(error.c_str());

        return defaults;
    }

    const uint32_t totalLeds =
        document["totalLeds"] | 0U;

    const uint8_t masterBrightness =
        document["masterBrightness"] | 255;

    if (totalLeds == 0 ||
        totalLeds > DeviceConfig::MAX_PIXEL_COUNT ||
        masterBrightness > 100) {
        Serial.println("Invalid dynamic settings. Using defaults.");
        return defaults;
    }

    JsonArrayConst segments =
        document["segments"].as<JsonArrayConst>();

    if (segments.isNull() ||
        segments.size() > DeviceConfig::MAX_SEGMENTS) {
        Serial.println("Invalid dynamic segment list. Using defaults.");
        return defaults;
    }

    DynamicDeviceSettings loaded =
        defaultDynamicDeviceSettings(totalLeds);

    loaded.version =
        document["version"] | DeviceConfig::SETTINGS_VERSION;

    loaded.masterBrightness = masterBrightness;

    for (JsonObjectConst item : segments) {
        DynamicSegmentSettings segment;

        segment.id = item["id"] | 0U;
        segment.name = item["name"] | "";
        segment.start = item["start"] | 0U;
        segment.count = item["count"] | 0U;
        segment.brightness = item["brightness"] | 100;
        segment.color = item["color"] | 0xFFFFFFU;
        segment.enabled = item["enabled"] | false;

        if (segment.id == 0 ||
            segment.count == 0 ||
            segment.start >= totalLeds ||
            segment.count > totalLeds - segment.start ||
            segment.brightness > 100 ||
            segment.color > 0xFFFFFFU) {
            Serial.println("Invalid segment data. Using defaults.");
            return defaults;
        }

        // Reject duplicate segment IDs.
        for (const auto& existing : loaded.segments) {
            if (existing.id == segment.id) {
                Serial.println("Duplicate segment ID. Using defaults.");
                return defaults;
            }
        }

        // Reject overlapping segment ranges.
        const uint32_t segmentEnd =
            segment.start + segment.count;

        for (const auto& existing : loaded.segments) {
            const uint32_t existingEnd =
                existing.start + existing.count;

            if (segment.start < existingEnd &&
                existing.start < segmentEnd) {
                Serial.println("Overlapping segments. Using defaults.");
                return defaults;
            }
        }

        loaded.segments.push_back(segment);
    }

    Serial.printf(
        "Loaded dynamic settings: %lu pixels, %u segments\n",
        static_cast<unsigned long>(loaded.totalLeds),
        static_cast<unsigned int>(loaded.segments.size())
    );

    return loaded;
}

// --------------------------------------------------
// DYNAMIC SETTINGS SAVE
// --------------------------------------------------

bool SettingsStore::saveDynamic(
    const DynamicDeviceSettings& settings
) {
    if (!initialized) {
        Serial.println("Cannot save settings: storage unavailable.");
        return false;
    }

    if (settings.totalLeds == 0 ||
        settings.totalLeds > DeviceConfig::MAX_PIXEL_COUNT ||
        settings.masterBrightness > 100 ||
        settings.segments.size() > DeviceConfig::MAX_SEGMENTS) {
        Serial.println("Cannot save settings: invalid configuration.");
        return false;
    }

    // Validate IDs, ranges, brightness and overlap before saving.
    for (size_t i = 0; i < settings.segments.size(); i++) {
        const auto& segment = settings.segments[i];

        if (segment.id == 0 ||
            segment.count == 0 ||
            segment.start >= settings.totalLeds ||
            segment.count > settings.totalLeds - segment.start ||
            segment.brightness > 100 ||
            segment.color > 0xFFFFFFU) {
            Serial.println("Cannot save settings: invalid segment.");
            return false;
        }

        for (size_t j = i + 1; j < settings.segments.size(); j++) {
            const auto& other = settings.segments[j];

            if (segment.id == other.id) {
                Serial.println("Cannot save settings: duplicate ID.");
                return false;
            }

            const uint32_t end = segment.start + segment.count;
            const uint32_t otherEnd = other.start + other.count;

            if (segment.start < otherEnd &&
                other.start < end) {
                Serial.println("Cannot save settings: overlapping segments.");
                return false;
            }
        }
    }

    JsonDocument document;

    document["version"] = DeviceConfig::SETTINGS_VERSION;
    document["totalLeds"] = settings.totalLeds;
    document["masterBrightness"] = settings.masterBrightness;

    JsonArray segments = document["segments"].to<JsonArray>();

    for (const auto& segment : settings.segments) {
        JsonObject item = segments.add<JsonObject>();

        item["id"] = segment.id;
        item["name"] = segment.name;
        item["start"] = segment.start;
        item["count"] = segment.count;
        item["brightness"] = segment.brightness;
        item["color"] = segment.color;
        item["enabled"] = segment.enabled;
    }

    String json;

    const size_t jsonSize = measureJson(document);

    if (jsonSize == 0) {
        Serial.println("Cannot save settings: JSON serialization failed.");
        return false;
    }

    serializeJson(document, json);

    const size_t written = preferences.putString(
        DYNAMIC_SETTINGS_KEY,
        json
    );

    if (written != json.length()) {
        Serial.println("Failed to write dynamic settings.");
        return false;
    }

    Serial.printf(
        "Saved dynamic settings: %lu pixels, %u segments, %u bytes\n",
        static_cast<unsigned long>(settings.totalLeds),
        static_cast<unsigned int>(settings.segments.size()),
        static_cast<unsigned int>(written)
    );

    return true;
}

// --------------------------------------------------
// CLOSE STORAGE
// --------------------------------------------------

void SettingsStore::close() {
    if (initialized) {
        preferences.end();
        initialized = false;
    }
}
