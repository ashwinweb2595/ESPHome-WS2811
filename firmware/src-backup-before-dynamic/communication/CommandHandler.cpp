#include "CommandHandler.h"

#include <ArduinoJson.h>

namespace {

constexpr int PROTOCOL_VERSION = 1;

bool parseHexColor(const char* text, uint32_t& color) {
    if (text == nullptr || strlen(text) != 7 || text[0] != '#') {
        return false;
    }

    for (int i = 1; i < 7; i++) {
        if (!isxdigit(static_cast<unsigned char>(text[i]))) {
            return false;
        }
    }

    color = strtoul(text + 1, nullptr, 16);
    return true;
}

String buildResponse(
    const String& id,
    bool ok,
    const char* message,
    const char* error = nullptr,
    JsonDocument* data = nullptr
) {
    JsonDocument response;

    response["protocol"] = PROTOCOL_VERSION;
    response["id"] = id;
    response["ok"] = ok;
    response["message"] = message;

    if (error != nullptr) {
        response["error"] = error;
    }

    if (data != nullptr) {
        response["data"] = data->as<JsonVariant>();
    } else {
        response["data"].to<JsonObject>();
    }

    String output;
    serializeJson(response, output);
    return output;
}

bool saveCurrentSettings(
    LedEngine& engine,
    SettingsStore& store
) {
    DeviceSettings settings = engine.getSettings();
    return store.save(settings);
}

} // namespace

CommandHandler::CommandHandler(
    LedEngine& engine,
    SettingsStore& store
) : ledEngine(engine), settingsStore(store) {
}

String CommandHandler::handle(const String& jsonCommand) {
    JsonDocument request;

    DeserializationError parseError =
        deserializeJson(request, jsonCommand);

    if (parseError) {
        return buildResponse(
            "",
            false,
            "Invalid JSON command",
            parseError.c_str()
        );
    }

    String id = request["id"] | "";

    int protocol = request["protocol"] | -1;

    if (protocol != PROTOCOL_VERSION) {
        return buildResponse(
            id,
            false,
            "Unsupported protocol version",
            "Expected protocol version 1"
        );
    }

    const char* type = request["type"] | "";

    JsonObjectConst payload =
        request["payload"].as<JsonObjectConst>();

    if (payload.isNull()) {
        return buildResponse(
            id,
            false,
            "Invalid command payload",
            "Payload must be a JSON object"
        );
    }

    JsonDocument data;

    // ------------------------------------------------
    // SET BRIGHTNESS
    // ------------------------------------------------

    if (strcmp(type, "set_brightness") == 0) {
        if (!payload["value"].is<int>()) {
            return buildResponse(
                id, false, "Invalid brightness",
                "Brightness value is required"
            );
        }

        int value = payload["value"].as<int>();

        if (value < 0 || value > 100) {
            return buildResponse(
                id, false, "Invalid brightness",
                "Brightness must be between 0 and 100"
            );
        }

        if (!payload["segment"].isNull()) {
            if (!payload["segment"].is<int>()) {
                return buildResponse(
                    id, false, "Invalid segment",
                    "Segment must be an integer"
                );
            }

            int segment = payload["segment"].as<int>();

            if (segment < 0 || segment >= DeviceConfig::SEGMENT_COUNT) {
                return buildResponse(
                    id, false, "Invalid segment",
                    "Segment must be between 0 and 5"
                );
            }

            ledEngine.setSegmentBrightness(segment, value);
        } else {
            ledEngine.setMasterBrightness(value);
        }

        if (!saveCurrentSettings(ledEngine, settingsStore)) {
            return buildResponse(
                id, false,
                "Brightness applied but not saved",
                "Unable to write settings to persistent storage"
            );
        }

        data["brightness"] = value;

        return buildResponse(
            id, true, "Brightness updated", nullptr, &data
        );
    }

    // ------------------------------------------------
    // SET COLOR
    // ------------------------------------------------

    if (strcmp(type, "set_color") == 0) {
        const char* colorText = payload["color"] | "";

        uint32_t color = 0;

        if (!parseHexColor(colorText, color)) {
            return buildResponse(
                id, false, "Invalid color",
                "Color must use #RRGGBB format"
            );
        }

        if (!payload["segment"].isNull()) {
            if (!payload["segment"].is<int>()) {
                return buildResponse(
                    id, false, "Invalid segment",
                    "Segment must be an integer"
                );
            }

            int segment = payload["segment"].as<int>();

            if (segment < 0 || segment >= DeviceConfig::SEGMENT_COUNT) {
                return buildResponse(
                    id, false, "Invalid segment",
                    "Segment must be between 0 and 5"
                );
            }

            ledEngine.setSegmentColor(segment, color);
            ledEngine.setSegmentEnabled(segment, true);
        } else {
            DeviceSettings settings = ledEngine.getSettings();

            for (uint8_t i = 0; i < DeviceConfig::SEGMENT_COUNT; i++) {
                settings.segments[i].color = color;
                settings.segments[i].enabled = true;
            }

            ledEngine.applySettings(settings);
        }

        if (!saveCurrentSettings(ledEngine, settingsStore)) {
            return buildResponse(
                id, false,
                "Color applied but not saved",
                "Unable to write settings to persistent storage"
            );
        }

        data["color"] = colorText;

        return buildResponse(
            id, true, "Color updated", nullptr, &data
        );
    }

    // ------------------------------------------------
    // SET INDIVIDUAL ADDRESSABLE SECTION
    // ------------------------------------------------

    if (strcmp(type, "set_led") == 0) {
        if (!payload["index"].is<int>()) {
            return buildResponse(
                id, false, "Invalid section index",
                "Index is required"
            );
        }

        int index = payload["index"].as<int>();

        if (index < 0 || index >= DeviceConfig::LED_COUNT) {
            return buildResponse(
                id, false, "Invalid section index",
                "Index must be between 0 and 65"
            );
        }

        uint32_t color = 0;
        const char* colorText = payload["color"] | "";

        if (!parseHexColor(colorText, color)) {
            return buildResponse(
                id, false, "Invalid color",
                "Color must use #RRGGBB format"
            );
        }

        if (!ledEngine.setLedColor(index, color)) {
            return buildResponse(
                id, false, "Unable to update section"
            );
        }

        data["index"] = index;
        data["color"] = colorText;

        return buildResponse(
            id, true, "Addressable section updated", nullptr, &data
        );
    }

    // ------------------------------------------------
    // GET STATUS
    // ------------------------------------------------

    if (strcmp(type, "get_status") == 0) {
        DeviceSettings settings = ledEngine.getSettings();

        data["device"] = "smart-led-controller";
        data["protocol"] = PROTOCOL_VERSION;
        data["led_count"] = DeviceConfig::LED_COUNT;
        data["segment_count"] = DeviceConfig::SEGMENT_COUNT;
        data["brightness"] = settings.masterBrightness;

        // Transport status will be populated when Wi-Fi
        // and BLE modules are connected to this handler.
        data["wifi_connected"] = false;
        data["bluetooth_connected"] = false;

        return buildResponse(
            id, true, "Device status retrieved", nullptr, &data
        );
    }

    // ------------------------------------------------
    // GET CONFIG
    // ------------------------------------------------

    if (strcmp(type, "get_config") == 0) {
        DeviceSettings settings = ledEngine.getSettings();

        data["version"] = settings.version;
        data["brightness"] = settings.masterBrightness;

        JsonArray segments = data["segments"].to<JsonArray>();

        for (uint8_t i = 0; i < DeviceConfig::SEGMENT_COUNT; i++) {
            const LedSegmentSettings& segment = settings.segments[i];

            JsonObject item = segments.add<JsonObject>();

            item["start"] = segment.start;
            item["end"] = segment.end;
            item["brightness"] = segment.brightness;
            item["enabled"] = segment.enabled != 0;

            char hex[8];
            snprintf(
                hex, sizeof(hex), "#%06lX",
                static_cast<unsigned long>(segment.color & 0xFFFFFF)
            );

            item["color"] = hex;
        }

        return buildResponse(
            id, true, "Configuration retrieved", nullptr, &data
        );
    }

    // ------------------------------------------------
    // SET CONFIG
    // ------------------------------------------------

    if (strcmp(type, "set_config") == 0) {
        JsonObjectConst config =
            payload["config"].as<JsonObjectConst>();

        if (config.isNull()) {
            return buildResponse(
                id, false, "Invalid configuration",
                "Config object is required"
            );
        }

        int brightness = config["brightness"] | -1;

        if (brightness < 0 || brightness > 100) {
            return buildResponse(
                id, false, "Invalid configuration",
                "Brightness must be between 0 and 100"
            );
        }

        JsonArrayConst segments =
            config["segments"].as<JsonArrayConst>();

        if (segments.size() != DeviceConfig::SEGMENT_COUNT) {
            return buildResponse(
                id, false, "Invalid configuration",
                "Exactly six segments are required"
            );
        }

        DeviceSettings settings = defaultDeviceSettings();
        settings.masterBrightness = brightness;

        for (uint8_t i = 0; i < DeviceConfig::SEGMENT_COUNT; i++) {
            JsonObjectConst item = segments[i].as<JsonObjectConst>();

            int start = item["start"] | -1;
            int end = item["end"] | -1;
            int segmentBrightness = item["brightness"] | -1;
            bool enabled = item["enabled"] | false;

            uint32_t color = 0;
            const char* colorText = item["color"] | "";

            if (start < 0 || start >= DeviceConfig::LED_COUNT ||
                end < 0 || end >= DeviceConfig::LED_COUNT ||
                start > end ||
                segmentBrightness < 0 || segmentBrightness > 100 ||
                !parseHexColor(colorText, color)) {
                return buildResponse(
                    id, false, "Invalid segment configuration",
                    "Check segment start, end, brightness and color"
                );
            }

            settings.segments[i].start = start;
            settings.segments[i].end = end;
            settings.segments[i].brightness = segmentBrightness;
            settings.segments[i].color = color;
            settings.segments[i].enabled = enabled ? 1 : 0;
        }

        ledEngine.applySettings(settings);

        if (!settingsStore.save(settings)) {
            return buildResponse(
                id, false,
                "Configuration applied but not saved",
                "Unable to write settings to persistent storage"
            );
        }

        return buildResponse(
            id, true, "Configuration updated and saved"
        );
    }

    // ------------------------------------------------
    // EFFECTS AND TRANSITIONS
    // Implemented in a later firmware module.
    // ------------------------------------------------

    if (strcmp(type, "set_effect") == 0) {
        return buildResponse(
            id, false, "Effect engine not implemented yet",
            "Animation support is planned for a later firmware module"
        );
    }

    if (strcmp(type, "set_transition") == 0) {
        return buildResponse(
            id, false, "Transition engine not implemented yet",
            "Gradual transition support is planned for a later firmware module"
        );
    }

    return buildResponse(
        id, false, "Unknown command",
        "Command type is not supported"
    );
}