#include "BleManager.h"

#include <string>

static constexpr char BLE_SERVICE_UUID[] =
    "7f510001-1b15-4f0a-9a3c-2d9e7b1a0001";

static constexpr char BLE_RX_UUID[] =
    "7f510002-1b15-4f0a-9a3c-2d9e7b1a0001";

static constexpr char BLE_TX_UUID[] =
    "7f510003-1b15-4f0a-9a3c-2d9e7b1a0001";

class BleServerEvents : public NimBLEServerCallbacks {
public:
    void onConnect(
        NimBLEServer* server,
        NimBLEConnInfo& connInfo
    ) override {
        Serial.println("BLE client connected.");
    }

    void onDisconnect(
        NimBLEServer* server,
        NimBLEConnInfo& connInfo,
        int reason
    ) override {
        Serial.printf("BLE client disconnected. Reason: %d\n", reason);

        if (server != nullptr) {
            server->startAdvertising();
            Serial.println("BLE advertising restarted.");
        }
    }
};

class BleRxCallbacks : public NimBLECharacteristicCallbacks {
public:
    BleRxCallbacks(
        CommandHandler& handler,
        NimBLECharacteristic*& tx
    ) : commandHandler(handler), txCharacteristic(tx) {
    }

    void onWrite(
        NimBLECharacteristic* characteristic,
        NimBLEConnInfo& connInfo
    ) override {
        if (characteristic == nullptr || txCharacteristic == nullptr) {
            return;
        }

        std::string received = characteristic->getValue();

        if (received.empty()) {
            Serial.println("BLE received an empty command.");
            return;
        }

        if (received.size() > 512) {
            Serial.println("BLE command rejected: payload too large.");
            return;
        }

        Serial.printf(
            "BLE command received: %u bytes\n",
            static_cast<unsigned int>(received.size())
        );

        String command(received.c_str());

        String response = commandHandler.handle(command);

        Serial.printf(
            "BLE response generated: %u bytes\n",
            static_cast<unsigned int>(response.length())
        );

        txCharacteristic->setValue(response.c_str());

        if (txCharacteristic->notify()) {
            Serial.println("BLE response notification sent.");
        } else {
            Serial.println(
                "BLE notification failed. Check connection, subscription and MTU."
            );
        }
    }

private:
    CommandHandler& commandHandler;
    NimBLECharacteristic*& txCharacteristic;
};

static BleServerEvents serverEvents;
static BleRxCallbacks* rxCallbacks = nullptr;

BleManager::BleManager(CommandHandler& handler)
    : commandHandler(handler) {
}

void BleManager::begin() {
    Serial.println("Initializing BLE...");

    NimBLEDevice::init("Smart LED Controller");
    NimBLEDevice::setMTU(247);

    server = NimBLEDevice::createServer();

    if (server == nullptr) {
        Serial.println("ERROR: BLE server creation failed.");
        return;
    }

    server->setCallbacks(&serverEvents, false);

    NimBLEService* service =
        server->createService(BLE_SERVICE_UUID);

    if (service == nullptr) {
        Serial.println("ERROR: BLE service creation failed.");
        return;
    }

    NimBLECharacteristic* rxCharacteristic =
        service->createCharacteristic(
            BLE_RX_UUID,
            NIMBLE_PROPERTY::WRITE |
            NIMBLE_PROPERTY::WRITE_NR,
            512
        );

    txCharacteristic =
        service->createCharacteristic(
            BLE_TX_UUID,
            NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::NOTIFY,
            512
        );

    if (rxCharacteristic == nullptr || txCharacteristic == nullptr) {
        Serial.println("ERROR: BLE characteristic creation failed.");
        return;
    }

    delete rxCallbacks;
    rxCallbacks = new BleRxCallbacks(
        commandHandler,
        txCharacteristic
    );

    rxCharacteristic->setCallbacks(rxCallbacks);

    service->start();

    NimBLEAdvertising* advertising = NimBLEDevice::getAdvertising();

    if (advertising == nullptr) {
        Serial.println("ERROR: BLE advertising unavailable.");
        return;
    }

    advertising->addServiceUUID(BLE_SERVICE_UUID);
    advertising->start();

    initialized = true;

    Serial.println("BLE service started.");
    Serial.println("BLE device name: Smart LED Controller");
    Serial.println("BLE advertising started.");
}

void BleManager::loop() {
    // NimBLE handles BLE events in its own task.
}