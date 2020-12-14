// ***************************
// |docname| - test BLE server
// ***************************
// This is based on `Neil Kolban example for IDF <https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp>`_. It was ported to the Arduino ESP32 by Evandro Copercini, with updates by chegewara.

// Includes
// ========
#include <string>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>


// Macros
// ======
//
// .. _UUIDs:
//
// UUIDs
// -----
// See `here <https://www.uuidgenerator.net/>`_ for generating UUIDs.
//
// The UUID for the CellBot service.
#define SERVICE_UUID ("6c533793-9bd6-47d6-8d3b-c10a704b6b97")
// Characteristic for pinMode.
#define PIN_MODE_CHARACTERISTIC_UUID ("6ea6d9b6-7b7e-451c-ab45-221298e43562")
// Characteristic for digitalWrite
#define DIGITAL_WRITE_CHARACTERISTIC_UUID ("d3423cf6-6da7-4dd8-a5ba-3c980c74bd6d")


class InvokeArduinoCallback: public BLECharacteristicCallbacks {
    public:
    // A buffer for messages; a C string.
    char buf[100];
    // A string to hold a copy of ``buf``.
    std::string ret;
    // The value read from a characteristic.
    std::string value;

    InvokeArduinoCallback() : ret(100, 0) {
        ret.assign("Not yet invoked.");
    };

    // On a read, return ``buf``.
    void onRead(BLECharacteristic* pCharacteristic) {
        pCharacteristic->setValue(ret.assign(buf));
    };

    // Get a write value and check its length.
    bool checkLength(size_t sz_expected_length, BLECharacteristic* pCharacteristic) {
        value = pCharacteristic->getValue();
        if (value.length() != sz_expected_length) {
            snprintf(buf, sizeof(buf), "Error: message length was %u, but expected %u.\n", value.length(), sz_expected_length);
            return false;
        }
        return true;
    };
};


// A write to this characteristic invokes ``pinMode``; results are reported by a read of this characteristic.
class PinModeCallback: public InvokeArduinoCallback {
    void onWrite(BLECharacteristic* pCharacteristic) {
        if (checkLength(2, pCharacteristic)) {
            snprintf(buf, sizeof(buf), "pinMode(%u, %u)\n", value[0], value[1]);
            pinMode(value[0], value[1]);
        }
    }
};


// A write to this characteristic invokes ``digitalWrite``; results are reported by a read of this characteristic.
class DigitalWriteCallback: public  InvokeArduinoCallback {
    void onWrite(BLECharacteristic* pCharacteristic) {
        if (checkLength(2, pCharacteristic)) {
            snprintf(buf, sizeof(buf), "digitalWrite(%u, %u)\n", value[0], value[1]);
            digitalWrite(value[0], value[1]);
        }
    };
};


// Functions
// =========
void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE work!");

    // Define the name visible when pairing this device.
    BLEDevice::init("CellBot");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);

    BLECharacteristic *pCharacteristic = pService->createCharacteristic(
        PIN_MODE_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristic->setCallbacks(new PinModeCallback());

    pCharacteristic = pService->createCharacteristic(
        DIGITAL_WRITE_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristic->setCallbacks(new DigitalWriteCallback());

    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    // Functions that help with iPhone connections issue. Why is this done twice?
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read it in your phone!");
}


void loop() {
    // put your main code here, to run repeatedly:
    delay(2000);
}
