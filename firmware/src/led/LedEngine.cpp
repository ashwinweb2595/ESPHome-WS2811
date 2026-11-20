
#include "LedEngine.h"

#include <algorithm>

void LedEngine::begin() {
    begin(66);
}

bool LedEngine::begin(uint32_t totalLeds) {
    if (initialized) {
        Serial.println("LED engine is already initialized.");
        return false;
    }

    if (totalLeds == 0 ||
        totalLeds > DeviceConfig::MAX_PIXEL_COUNT) {
        Serial.println("Invalid LED count.");
        return false;
    }

    pixelCount = totalLeds;

    leds.assign(pixelCount, CRGB::Black);
    ledOverrides.assign(pixelCount, 0);
    overrideEnabled.assign(pixelCount, 0);

    FastLED.addLeds<WS2811, DeviceConfig::LED_PIN, GRB>(
        leds.data(),
        pixelCount
    );

    FastLED.setBrightness(255);
    FastLED.clear(true);

    currentSettings = defaultDynamicDeviceSettings(pixelCount);

    initialized = true;

    Serial.printf(
        "Dynamic LED engine initialized: %lu pixels\n",
        static_cast<unsigned long>(pixelCount)
    );

    return true;
}

// --------------------------------------------------
// LEGACY CONFIGURATION COMPATIBILITY
// --------------------------------------------------

void LedEngine::applySettings(const DeviceSettings& settings) {
    if (!initialized) return;

    DynamicDeviceSettings dynamicSettings =
        defaultDynamicDeviceSettings(pixelCount);

    dynamicSettings.masterBrightness =
        constrain(settings.masterBrightness, 0, 100);

    for (uint8_t i = 0; i < DeviceConfig::SEGMENT_COUNT; i++) {
        const LedSegmentSettings& oldSegment = settings.segments[i];

        if (oldSegment.start > oldSegment.end ||
            oldSegment.start >= pixelCount ||
            oldSegment.end >= pixelCount) {
            continue;
        }

        DynamicSegmentSettings segment;

        segment.id = i + 1;
        segment.name = "Segment " + String(i + 1);
        segment.start = oldSegment.start;
        segment.count =
            static_cast<uint32_t>(oldSegment.end) -
            oldSegment.start + 1;

        segment.brightness =
            constrain(oldSegment.brightness, 0, 100);

        segment.color = oldSegment.color & 0xFFFFFF;
        segment.enabled = oldSegment.enabled != 0;

        dynamicSettings.segments.push_back(segment);
    }

    applyDynamicSettings(dynamicSettings);
}

DeviceSettings LedEngine::getSettings() const {
    DeviceSettings legacy = defaultDeviceSettings();

    legacy.masterBrightness = currentSettings.masterBrightness;

    const size_t count = std::min(
        currentSettings.segments.size(),
        static_cast<size_t>(DeviceConfig::SEGMENT_COUNT)
    );

    for (size_t i = 0; i < count; i++) {
        const auto& segment = currentSettings.segments[i];

        uint32_t end = segment.start + segment.count - 1;

        legacy.segments[i].start =
            static_cast<uint8_t>(std::min<uint32_t>(segment.start, 255));

        legacy.segments[i].end =
            static_cast<uint8_t>(std::min<uint32_t>(end, 255));

        legacy.segments[i].brightness = segment.brightness;
        legacy.segments[i].color = segment.color;
        legacy.segments[i].enabled = segment.enabled ? 1 : 0;
    }

    return legacy;
}

// --------------------------------------------------
// DYNAMIC CONFIGURATION
// --------------------------------------------------

bool LedEngine::validateSettings(
    const DynamicDeviceSettings& settings
) const {
    if (settings.totalLeds != pixelCount ||
        settings.totalLeds == 0 ||
        settings.masterBrightness > 100 ||
        settings.segments.size() > DeviceConfig::MAX_SEGMENTS) {
        return false;
    }

    for (size_t i = 0; i < settings.segments.size(); i++) {
        const auto& segment = settings.segments[i];

        if (segment.id == 0 ||
            segment.count == 0 ||
            segment.start >= pixelCount ||
            segment.count > pixelCount - segment.start ||
            segment.brightness > 100) {
            return false;
        }

        // IDs must be unique.
        for (size_t j = i + 1; j < settings.segments.size(); j++) {
            if (segment.id == settings.segments[j].id) {
                return false;
            }
        }

        // Segments must not overlap.
        const uint32_t end = segment.start + segment.count;

        for (size_t j = i + 1; j < settings.segments.size(); j++) {
            const auto& other = settings.segments[j];
            const uint32_t otherEnd = other.start + other.count;

            if (segment.start < otherEnd &&
                other.start < end) {
                return false;
            }
        }
    }

    return true;
}

bool LedEngine::applyDynamicSettings(
    const DynamicDeviceSettings& settings
) {
    if (!initialized || !validateSettings(settings)) {
        return false;
    }

    currentSettings = settings;
    render();

    return true;
}

DynamicDeviceSettings LedEngine::getDynamicSettings() const {
    return currentSettings;
}

// --------------------------------------------------
// MASTER BRIGHTNESS
// --------------------------------------------------

void LedEngine::setMasterBrightness(uint8_t brightness) {
    if (!initialized) return;

    currentSettings.masterBrightness =
        constrain(brightness, 0, 100);

    render();
}

// --------------------------------------------------
// SEGMENT LOOKUP
// --------------------------------------------------

DynamicSegmentSettings* LedEngine::findSegment(uint32_t id) {
    for (auto& segment : currentSettings.segments) {
        if (segment.id == id) {
            return &segment;
        }
    }

    return nullptr;
}

// --------------------------------------------------
// LEGACY SEGMENT OPERATIONS BY INDEX
// --------------------------------------------------

bool LedEngine::setSegmentColor(
    uint8_t segment,
    uint32_t color
) {
    if (segment >= currentSettings.segments.size()) {
        return false;
    }

    return setSegmentColorById(
        currentSettings.segments[segment].id,
        color
    );
}

bool LedEngine::setSegmentBrightness(
    uint8_t segment,
    uint8_t brightness
) {
    if (segment >= currentSettings.segments.size()) {
        return false;
    }

    return setSegmentBrightnessById(
        currentSettings.segments[segment].id,
        brightness
    );
}

bool LedEngine::setSegmentEnabled(
    uint8_t segment,
    bool enabled
) {
    if (segment >= currentSettings.segments.size()) {
        return false;
    }

    return setSegmentEnabledById(
        currentSettings.segments[segment].id,
        enabled
    );
}

// --------------------------------------------------
// DYNAMIC SEGMENT OPERATIONS BY ID
// --------------------------------------------------

bool LedEngine::setSegmentColorById(
    uint32_t id,
    uint32_t color
) {
    auto* segment = findSegment(id);

    if (segment == nullptr) return false;

    segment->color = color & 0xFFFFFF;

    render();
    return true;
}

bool LedEngine::setSegmentBrightnessById(
    uint32_t id,
    uint8_t brightness
) {
    auto* segment = findSegment(id);

    if (segment == nullptr || brightness > 100) {
        return false;
    }

    segment->brightness = brightness;

    render();
    return true;
}

bool LedEngine::setSegmentEnabledById(
    uint32_t id,
    bool enabled
) {
    auto* segment = findSegment(id);

    if (segment == nullptr) return false;

    segment->enabled = enabled;

    render();
    return true;
}

// --------------------------------------------------
// ADD SEGMENT
// --------------------------------------------------

bool LedEngine::addSegment(
    const DynamicSegmentSettings& segment
) {
    if (!initialized ||
        currentSettings.segments.size() >= DeviceConfig::MAX_SEGMENTS) {
        return false;
    }

    DynamicDeviceSettings candidate = currentSettings;
    candidate.segments.push_back(segment);

    if (!validateSettings(candidate)) {
        return false;
    }

    currentSettings = candidate;
    render();

    return true;
}

// --------------------------------------------------
// UPDATE SEGMENT
// --------------------------------------------------

bool LedEngine::updateSegment(
    const DynamicSegmentSettings& segment
) {
    if (!initialized) return false;

    DynamicDeviceSettings candidate = currentSettings;

    bool found = false;

    for (auto& existing : candidate.segments) {
        if (existing.id == segment.id) {
            existing = segment;
            found = true;
            break;
        }
    }

    if (!found || !validateSettings(candidate)) {
        return false;
    }

    currentSettings = candidate;
    render();

    return true;
}

// --------------------------------------------------
// DELETE SEGMENT
// --------------------------------------------------

bool LedEngine::removeSegment(uint32_t id) {
    if (!initialized) return false;

    auto& segments = currentSettings.segments;

    auto it = std::find_if(
        segments.begin(),
        segments.end(),
        [id](const DynamicSegmentSettings& segment) {
            return segment.id == id;
        }
    );

    if (it == segments.end()) {
        return false;
    }

    segments.erase(it);

    render();
    return true;
}

// --------------------------------------------------
// INDIVIDUAL ADDRESSABLE PIXEL
// --------------------------------------------------

bool LedEngine::setLedColor(
    uint32_t index,
    uint32_t color
) {
    if (!initialized || index >= pixelCount) {
        return false;
    }

    ledOverrides[index] = color & 0xFFFFFF;
    overrideEnabled[index] = 1;

    render();
    return true;
}

// --------------------------------------------------
// CLEAR
// --------------------------------------------------

void LedEngine::clear() {
    if (!initialized) return;

    std::fill(
        overrideEnabled.begin(),
        overrideEnabled.end(),
        0
    );

    for (auto& segment : currentSettings.segments) {
        segment.enabled = false;
    }

    render();
}

void LedEngine::show() {
    render();
}

// --------------------------------------------------
// RENDER
// --------------------------------------------------

void LedEngine::render() {
    if (!initialized || pixelCount == 0) return;

    fill_solid(
        leds.data(),
        pixelCount,
        CRGB::Black
    );

    const uint8_t master = currentSettings.masterBrightness;

    for (const auto& segment : currentSettings.segments) {
        if (!segment.enabled) continue;

        CRGB color(
            (segment.color >> 16) & 0xFF,
            (segment.color >> 8) & 0xFF,
            segment.color & 0xFF
        );

        const uint8_t brightness =
            (static_cast<uint16_t>(master) *
             segment.brightness) / 100;

        color.nscale8_video(
            (static_cast<uint16_t>(brightness) * 255) / 100
        );

        const uint32_t end = segment.start + segment.count;

        for (uint32_t i = segment.start; i < end; i++) {
            leds[i] = color;
        }
    }

    // Individual pixel overrides take priority over segment colours.
    for (uint32_t i = 0; i < pixelCount; i++) {
        if (!overrideEnabled[i]) continue;

        CRGB color(
            (ledOverrides[i] >> 16) & 0xFF,
            (ledOverrides[i] >> 8) & 0xFF,
            ledOverrides[i] & 0xFF
        );

        color.nscale8_video(
            (static_cast<uint16_t>(master) * 255) / 100
        );

        leds[i] = color;
    }

    FastLED.show();
}
