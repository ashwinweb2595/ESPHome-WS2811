#pragma once

#include <Arduino.h>
#include "../led/LedEngine.h"
#include "../storage/SettingsStore.h"

class CommandHandler {
public:
    CommandHandler(LedEngine& engine, SettingsStore& store);

    String handle(const String& jsonCommand);

private:
    LedEngine& ledEngine;
    SettingsStore& settingsStore;
};