#include "LedEngine.h"
#include <FastLED.h>

static CRGB leds[DeviceConfig::LED_COUNT];

void LedEngine::begin() {
    FastLED.addLeds<WS2811, DeviceConfig::LED_PIN, GRB>(
        leds, DeviceConfig::LED_COUNT
    );

    FastLED.setBrightness(255);
    FastLED.clear(true);
    currentSettings = defaultDeviceSettings();
}

void LedEngine::applySettings(const DeviceSettings& settings) {
    currentSettings = settings;
    render();
}

void LedEngine::setMasterBrightness(uint8_t brightness) {
    currentSettings.masterBrightness = constrain(brightness, 0, 100);
    render();
}

bool LedEngine::setSegmentColor(uint8_t segment, uint32_t color) {
    if (segment >= DeviceConfig::SEGMENT_COUNT) return false;

    currentSettings.segments[segment].color = color & 0xFFFFFF;
    render();
    return true;
}

bool LedEngine::setSegmentBrightness(uint8_t segment, uint8_t brightness) {
    if (segment >= DeviceConfig::SEGMENT_COUNT) return false;

    currentSettings.segments[segment].brightness =
        constrain(brightness, 0, 100);
    render();
    return true;
}

bool LedEngine::setSegmentEnabled(uint8_t segment, bool enabled) {
    if (segment >= DeviceConfig::SEGMENT_COUNT) return false;

    currentSettings.segments[segment].enabled = enabled ? 1 : 0;
    render();
    return true;
}

bool LedEngine::setLedColor(uint8_t index, uint32_t color) {
    if (index >= DeviceConfig::LED_COUNT) return false;

    ledOverrides[index] = color & 0xFFFFFF;
    overrideEnabled[index] = true;
    render();
    return true;
}

void LedEngine::clear() {
    for (uint16_t i = 0; i < DeviceConfig::LED_COUNT; i++) {
        overrideEnabled[i] = false;
    }

    for (uint8_t i = 0; i < DeviceConfig::SEGMENT_COUNT; i++) {
        currentSettings.segments[i].enabled = false;
    }

    render();
}

void LedEngine::show() {
    render();
}

void LedEngine::render() {
    fill_solid(leds, DeviceConfig::LED_COUNT, CRGB::Black);

    const uint8_t master = currentSettings.masterBrightness;

    for (uint8_t s = 0; s < DeviceConfig::SEGMENT_COUNT; s++) {
        const LedSegmentSettings& segment = currentSettings.segments[s];

        if (!segment.enabled) continue;

        const uint8_t start = segment.start;
        const uint8_t end = min(
            static_cast<uint16_t>(segment.end),
            static_cast<uint16_t>(DeviceConfig::LED_COUNT - 1)
        );

        if (start > end) continue;

        CRGB color(
            (segment.color >> 16) & 0xFF,
            (segment.color >> 8) & 0xFF,
            segment.color & 0xFF
        );

        const uint8_t brightness =
            (static_cast<uint16_t>(master) * segment.brightness) / 100;

        color.nscale8_video(
            (static_cast<uint16_t>(brightness) * 255) / 100
        );

        for (uint16_t i = start; i <= end; i++) {
            leds[i] = color;
        }
    }

    for (uint16_t i = 0; i < DeviceConfig::LED_COUNT; i++) {
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