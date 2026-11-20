
#include "CommandHandler.h"

#include <ArduinoJson.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

namespace {

constexpr int PROTOCOL_VERSION = 1;

// --------------------------------------------------
// RESPONSE HELPER
// --------------------------------------------------

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

// --------------------------------------------------
// COLOR PARSER
// --------------------------------------------------

bool parseHexColor(const char* text, uint32_t& color) {
    if (text == nullptr ||
        strlen(text) != 7 ||
        text[0] != '#') {
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

// --------------------------------------------------
// SAVE DYNAMIC SETTINGS
// --------------------------------------------------

bool saveCurrentSettings(
    LedEngine& engine,
    SettingsStore& store
) {
    DynamicDeviceSettings settings =
        engine.getDynamicSettings();

    return store.saveDynamic(settings);
}

// --------------------------------------------------
// SEGMENT PARSER
// --------------------------------------------------

bool parseSegment(
    JsonObjectConst item,
    DynamicSegmentSettings& segment
) {
    if (item.isNull()) {
        return false;
    }

    if (!item["id"].is<uint32_t>() ||
        !item["start"].is<uint32_t>() ||
        !item["brightness"].is<int>() ||
        !item["enabled"].is<bool>()) {
        return false;
    }

    segment.id = item["id"].as<uint32_t>();
    segment.start = item["start"].as<uint32_t>();

    int brightness = item["brightness"].as<int>();

    if (brightness < 0 || brightness > 100) {
        return false;
    }

    segment.brightness = brightness;

    segment.name = item["name"] | "";

    if (segment.name.length() > 32) {
        return false;
    }

    segment.enabled = item["enabled"].as<bool>();

    // New format: start + count.
    if (item["count"].is<uint32_t>()) {
        segment.count = item["count"].as<uint32_t>();
    }
    // Legacy format: start + end.
    else if (item["end"].is<uint32_t>()) {
        uint32_t end = item["end"].as<uint32_t>();

        if (end < segment.start) {
            return false;
        }

        segment.count = end - segment.start + 1;
    } else {
        return false;
    }

    const char* colorText = item["color"] | "";

    if (!parseHexColor(colorText, segment.color)) {
        return false;
    }

    return true;
}

// --------------------------------------------------
// ADD SEGMENT TO RESPONSE
// --------------------------------------------------

void addSegmentToJson(
    JsonArray array,
    const DynamicSegmentSettings& segment
) {
    JsonObject item = array.add<JsonObject>();

    item["id"] = segment.id;
    item["name"] = segment.name;
    item["start"] = segment.start;
    item["count"] = segment.count;

    // Retain end for compatibility with older clients.
    item["end"] = segment.start + segment.count - 1;

    item["brightness"] = segment.brightness;
    item["enabled"] = segment.enabled;

    char hex[8];

    snprintf(
        hex,
        sizeof(hex),
        "#%06lX",
        static_cast<unsigned long>(segment.color & 0xFFFFFF)
    );

    item["color"] = hex;
}

// --------------------------------------------------
// PERSIST AFTER CHANGE, ROLL BACK IF SAVE FAILS
// --------------------------------------------------

bool persistOrRollback(
    LedEngine& engine,
    SettingsStore& store,
    const DynamicDeviceSettings& previous
) {
    if (saveCurrentSettings(engine, store)) {
        return true;
    }

    engine.applyDynamicSettings(previous);

    return false;
}

} // namespace

// ==================================================
// COMMAND HANDLER
// ==================================================

CommandHandler::CommandHandler(
    LedEngine& engine,
    SettingsStore& store
) : ledEngine(engine), settingsStore(store) {
}

// ==================================================
// HANDLE COMMAND
// ==================================================

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
     const char* type       id,
            false,
            "Unsupported protocol version",
            "Expected protocol version 1"
        );
    }

     = request["type"] | "";

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

    // ==================================================
    // SET BRIGHTNESS
    // ==================================================

    if (strcmp(type, "set_brightness") == 0) {
        if (!payload["value"].is<int>()) {
            return buildResponse(
                id, false,
                "Invalid brightness",
                "Brightness value is required"
            );
        }

        int value = payload["value"].as<int>();

        if (value < 0 || value > 100) {
            return buildResponse(
                id, false,
                "Invalid brightness",
                "Brightness must be between 0 and 100"
            );
        }

        DynamicDeviceSettings previous =
            ledEngine.getDynamicSettings();

        bool updated = false;

        // New API: segment_id.
        if (!payload["segment_id"].isNull()) {
            if (!payload["segment_id"].is<uint32_t>()) {
                return buildResponse(
                    id, false,
                    "Invalid segment ID",
                    "Segment ID must be a positive integer"
                );
            }

            updated = ledEngine.setSegmentBrightnessById(
                payload["segment_id"].as<uint32_t>(),
                value
            );
        }
        // Legacy API: segment array index.
        else if (!payload["segment"].isNull()) {
            if (!payload["segment"].is<int>()) {
                return buildResponse(
                    id, false,
                    "Invalid segment",
                    "Segment must be an integer"
                );
            }

            int segment = payload["segment"].as<int>();

            if (segment < 0 ||
                segment >= previous.segments.size()) {
                return buildResponse(
                    id, false,
                    "Invalid segment",
                    "Segment index does not exist"
                );
            }

            updated = ledEngine.setSegmentBrightness(
                segment,
                value
            );
        }
        // No segment selector: master brightness.
        else {
            ledEngine.setMasterBrightness(value);
            updated = true;
        }

        if (!updated) {
            return buildResponse(
                id, false,
                "Unable to update brightness",
                "Segment was not found or value was invalid"
            );
        }

        if (!persistOrRollback(
                ledEngine,
                settingsStore,
                previous
            )) {
            return buildResponse(
                id, false,
                "Brightness applied but not saved",
                "Unable to write settings to persistent storage"
            );
        }

        data["brightness"] = value;

        return buildResponse(
            id, true,
            "Brightness updated",
            nullptr,
            &data
        );
    }

    // ==================================================
    // SET COLOR
    // ==================================================

    if (strcmp(type, "set_color") == 0) {
        const char* colorText = payload["color"] | "";

        uint32_t color = 0;

        if (!parseHexColor(colorText, color)) {
            return buildResponse(
                id, false,
                "Invalid color",
                "Color must use #RRGGBB format"
            );
        }

        DynamicDeviceSettings previous =
            ledEngine.getDynamicSettings();

        bool updated = false;

        // New API: segment_id.
        if (!payload["segment_id"].isNull()) {
            if (!payload["segment_id"].is<uint32_t>()) {
                return buildResponse(
                    id, false,
                    "Invalid segment ID",
                    "Segment ID must be a positive integer"
                );
            }

            uint32_t segmentId =
                payload["segment_id"].as<uint32_t>();

            updated = ledEngine.setSegmentColorById(
                segmentId,
                color
            );

            if (updated) {
                ledEngine.setSegmentEnabledById(
                    segmentId,
                    true
                );
            }
        }
        // Legacy API: segment array index.
        else if (!payload["segment"].isNull()) {
            if (!payload["segment"].is<int>()) {
                return buildResponse(
                    id, false,
                    "Invalid segment",
                    "Segment must be an integer"
                );
            }

            int segment = payload["segment"].as<int>();

            if (segment < 0 ||
                segment >= previous.segments.size()) {
                return buildResponse(
                    id, false,
                    "Invalid segment",
                    "Segment index does not exist"
                );
            }

            updated = ledEngine.setSegmentColor(
                segment,
                color
            );

            if (updated) {
                ledEngine.setSegmentEnabled(
                    segment,
                    true
                );
            }
        }
        // No selector: apply to all segments.
        else {
            DynamicDeviceSettings updatedSettings = previous;

            for (auto& segment : updatedSettings.segments) {
                segment.color = color;
                segment.enabled = true;
            }

            updated = ledEngine.applyDynamicSettings(
                updatedSettings
            );
        }

        if (!updated) {
            return buildResponse(
                id, false,
                "Unable to update color",
                "Segment was not found"
            );
        }

        if (!persistOrRollback(
                ledEngine,
                settingsStore,
                previous
            )) {
            return buildResponse(
                id, false,
                "Color applied but not saved",
                "Unable to write settings to persistent storage"
            );
        }

        data["color"] = colorText;

        return buildResponse(
            id, true,
            "Color updated",
            nullptr,
            &data
        );
    }

    // ==================================================
    // SET INDIVIDUAL ADDRESSABLE PIXEL
    // ==================================================

    if (strcmp(type, "set_led") == 0) {
        if (!payload["index"].is<int>()) {
            return buildResponse(
                id, false,
                "Invalid pixel index",
                "Index is required"
            );
        }

        int index = payload["index"].as<int>();

        if (index < 0 ||
            static_cast<uint32_t>(index) >= ledEngine.getPixelCount()) {
            return buildResponse(
                id, false,
                "Invalid pixel index",
                "Index is outside the configured pixel range"
            );
        }

        uint32_t color = 0;

        const char* colorText = payload["color"] | "";

        if (!parseHexColor(colorText, color)) {
            return buildResponse(
                id, false,
                "Invalid color",
                "Color must use #RRGGBB format"
            );
        }

        if (!ledEngine.setLedColor(index, color)) {
            return buildResponse(
                id, false,
                "Unable to update pixel"
            );
        }

        data["index"] = index;
        data["color"] = colorText;

        return buildResponse(
            id, true,
            "Addressable pixel updated",
            nullptr,
            &data
        );
    }

    // ==================================================
    // GET STATUS
    // ==================================================

    if (strcmp(type, "get_status") == 0) {
        DynamicDeviceSettings settings =
            ledEngine.getDynamicSettings();

        data["device"] = "smart-led-controller";
        data["protocol"] = PROTOCOL_VERSION;

        data["led_count"] = ledEngine.getPixelCount();
        data["segment_count"] = settings.segments.size();

        data["brightness"] = settings.masterBrightness;

        data["wifi_connected"] = false;
        data["bluetooth_connected"] = false;

        return buildResponse(
            id, true,
            "Device status retrieved",
            nullptr,
            &data
        );
    }

    // ==================================================
    // GET CONFIG
    // ==================================================

    if (strcmp(type, "get_config") == 0) {
        DynamicDeviceSettings settings =
            ledEngine.getDynamicSettings();

        data["version"] = settings.version;
        data["totalLeds"] = settings.totalLeds;
        data["brightness"] = settings.masterBrightness;

        JsonArray segments =
            data["segments"].to<JsonArray>();

        for (const auto& segment : settings.segments) {
            addSegmentToJson(segments, segment);
        }

        return buildResponse(
            id, true,
            "Configuration retrieved",
            nullptr,
            &data
        );
    }

    // ==================================================
    // SET CONFIG
    // ==================================================

    if (strcmp(type, "set_config") == 0) {
        JsonObjectConst config =
            payload["config"].as<JsonObjectConst>();

        if (config.isNull()) {
            return buildResponse(
                id, false,
                "Invalid configuration",
                "Config object is required"
            );
        }

        DynamicDeviceSettings candidate =
            defaultDynamicDeviceSettings(
                ledEngine.getPixelCount()
            );

        candidate.masterBrightness =
            config["brightness"] | 255;

        candidate.version =
            DeviceConfig::SETTINGS_VERSION;

        // Total pixel count must match the initialized engine for now.
        uint32_t requestedTotalLeds =
            config["totalLeds"] | ledEngine.getPixelCount();

        if (requestedTotalLeds != ledEngine.getPixelCount()) {
            return buildResponse(
                id, false,
                "Pixel count change requires restart support",
                "The configured totalLeds must match the currently initialized LED engine"
            );
        }

        if (candidate.masterBrightness > 100) {
            return buildResponse(
                id, false,
                "Invalid configuration",
                "Brightness must be between 0 and 100"
            );
        }

        JsonArrayConst segments =
            config["segments"].as<JsonArrayConst>();

        if (segments.isNull() ||
            segments.size() > DeviceConfig::MAX_SEGMENTS) {
            return buildResponse(
                id, false,
                "Invalid configuration",
                "Segment list is missing or exceeds the supported limit"
            );
        }

        for (JsonObjectConst item : segments) {
            DynamicSegmentSettings segment;

            if (!parseSegment(item, segment)) {
                return buildResponse(
                    id, false,
                    "Invalid segment configuration",
                    "Check ID, name, start, count, brightness, enabled and color"
                );
            }

            candidate.segments.push_back(segment);
        }

        DynamicDeviceSettings previous =
            ledEngine.getDynamicSettings();

        if (!ledEngine.applyDynamicSettings(candidate)) {
            return buildResponse(
                id, false,
                "Invalid configuration",
                "Check segment IDs, ranges and overlapping segments"
            );
        }

        if (!settingsStore.saveDynamic(candidate)) {
            ledEngine.applyDynamicSettings(previous);

            return buildResponse(
                id, false,
                "Configuration applied but not saved",
                "Unable to write settings to persistent storage"
            );
        }

        return buildResponse(
            id, true,
            "Dynamic configuration updated and saved"
        );
    }

    // ==================================================
    // ADD SEGMENT
    // ==================================================

    if (strcmp(type, "add_segment") == 0) {
        DynamicSegmentSettings segment;

        JsonObjectConst item =
            payload["segment"].as<JsonObjectConst>();

        if (!parseSegment(item, segment)) {
            return buildResponse(
                id, false,
                "Invalid segment",
                "A valid segment object is required"
            );
        }

        DynamicDeviceSettings previous =
            ledEngine.getDynamicSettings();

        if (!ledEngine.addSegment(segment)) {
            return buildResponse(
                id, false,
                "Unable to add segment",
                "Check segment ID, range, overlap and maximum segment count"
            );
        }

        if (!persistOrRollback(
                ledEngine,
                settingsStore,
                previous
            )) {
            return buildResponse(
                id, false,
                "Segment added but not saved",
                "Unable to write settings to persistent storage"
            );
        }

        data["segment_id"] = segment.id;

        return buildResponse(
            id, true,
            "Segment added",
            nullptr,
            &data
        );
    }

    // ==================================================
    // UPDATE SEGMENT
    // ==================================================

    if (strcmp(type, "update_segment") == 0) {
        DynamicSegmentSettings segment;

        JsonObjectConst item =
            payload["segment"].as<JsonObjectConst>();

        if (!parseSegment(item, segment)) {
            return buildResponse(
                id, false,
                "Invalid segment",
                "A valid segment object is required"
            );
        }

        DynamicDeviceSettings previous =
            ledEngine.getDynamicSettings();

        if (!ledEngine.updateSegment(segment)) {
            return buildResponse(
                id, false,
                "Unable to update segment",
                "Segment ID was not found or its configuration is invalid"
            );
        }

        if (!persistOrRollback(
                ledEngine,
                settingsStore,
                previous
            )) {
            return buildResponse(
                id, false,
                "Segment updated but not saved",
                "Unable to write settings to persistent storage"
            );
        }

        data["segment_id"] = segment.id;

        return buildResponse(
            id, true,
            "Segment updated",
            nullptr,
            &data
        );
    }

    // ==================================================
    // DELETE SEGMENT
    // ==================================================

    if (strcmp(type, "delete_segment") == 0) {
        if (!payload["segment_id"].is<uint32_t>()) {
            return buildResponse(
                id, false,
                "Invalid segment ID",
                "segment_id is required"
            );
        }

        uint32_t segmentId =
            payload["segment_id"].as<uint32_t>();

        DynamicDeviceSettings previous =
            ledEngine.getDynamicSettings();

        if (!ledEngine.removeSegment(segmentId)) {
            return buildResponse(
                id, false,
                "Unable to delete segment",
                "Segment ID was not found"
            );
        }

        if (!persistOrRollback(
                ledEngine,
                settingsStore,
                previous
            )) {
            return buildResponse(
                id, false,
                "Segment deleted but not saved",
                "Unable to write settings to persistent storage"
            );
        }

        data["segment_id"] = segmentId;

        return buildResponse(
            id, true,
            "Segment deleted",
            nullptr,
            &data
        );
    }

    // ==================================================
    // EFFECTS
    // ==================================================

    if (strcmp(type, "set_effect") == 0) {
        return buildResponse(
            id, false,
            "Effect engine not implemented yet",
            "Animation support is planned for a later firmware module"
        );
    }

    // ==================================================
    // TRANSITIONS
    // ==================================================

    if (strcmp(type, "set_transition") == 0) {
        return buildResponse(
            id, false,
            "Transition engine not implemented yet",
            "Gradual transition support is planned for a later firmware module"
        );
    }

    // ==================================================
    // UNKNOWN COMMAND
    // ==================================================

    return buildResponse(
        id, false,
        "Unknown command",
        "Command type is not supported"
    );
}
