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

// To wrap:
//
// -    ``void ledcAttachPin(uint8_t pin, uint8_t channel)``
//
//
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
// Characteristic for digitalRead
#define DIGITAL_READ_CHARACTERISTIC_UUID ("c370bc79-11c1-4530-9f69-ab9d961aa497")
// Characteristic for ledcSetup
#define LEDC_SETUP_CHARACTERISTIC_UUID ("6be57cea-3c46-4687-972b-03429d2acf9b")
// Characteristic for ledcWrite
#define LEDC_WRITE_CHARACTERISTIC_UUID ("40698030-a343-448f-a9ea-54b39b03bf81")


class InvokeArduinoCallback: public BLECharacteristicCallbacks {
    public:
    // A buffer for messages; a C string.
    char buf[100];
    // A string holding read data to return to the client.
    std::string ret;
    // The value read from a characteristic.
    std::string value;

    InvokeArduinoCallback() : ret(100, 0) {
        ret.assign("Not yet invoked.");
    };

    // On a read, return ``buf``.
    void onRead(BLECharacteristic* pCharacteristic) {
        pCharacteristic->setValue(ret);
    };

    // Get a write value and check its length.
    bool checkLength(size_t sz_expected_length, BLECharacteristic* pCharacteristic) {
        value = pCharacteristic->getValue();
        if (value.length() != sz_expected_length) {
            snprintf(buf, sizeof(buf), "Error: message length was %u, but expected %u.\n", value.length(), sz_expected_length);
            ret.assign(buf);
            return false;
        }
        return true;
    };
};


// A write to this characteristic invokes ``pinMode``; results are reported by a read of this characteristic.
class PinModeCallback: public InvokeArduinoCallback {
    void onWrite(BLECharacteristic* pCharacteristic) {
        ///                 value[0]    value[1]
        /// void pinMode(uint8_t pin, uint8_t mode)
        uint8_t u8_pin;
        uint8_t u8_mode;

        if (checkLength(2, pCharacteristic)) {
            u8_pin = static_cast<uint8_t>(value[0]);
            u8_mode = static_cast<uint8_t>(value[1]);
            snprintf(buf, sizeof(buf), "pinMode(%u, %u)\n", u8_pin, u8_mode);
            ret.assign(buf);
            pinMode(u8_pin, u8_mode);
        }
    }
};


// A write to this characteristic invokes ``digitalWrite``; results are reported by a read of this characteristic.
class DigitalWriteCallback: public InvokeArduinoCallback {
    void onWrite(BLECharacteristic* pCharacteristic) {
        ///                     value[0]     value[1]
        /// void digitalWrite(uint8_t pin, uint8_t val);
        uint8_t u8_pin;
        uint8_t u8_val;

        if (checkLength(2, pCharacteristic)) {
            u8_pin = static_cast<uint8_t>(value[0]);
            u8_val = static_cast<uint8_t>(value[1]);
            snprintf(buf, sizeof(buf), "digitalWrite(%u, %u)\n", u8_pin, u8_val);
            ret.assign(buf);
            digitalWrite(u8_pin, u8_val);
        }
    };
};


// A write to this characteristic invokes ``digitalWrite``; results are reported by a read of this characteristic.
class DigitalReadCallback: public InvokeArduinoCallback {
    void onWrite(BLECharacteristic* pCharacteristic) {
        /// ret[0]               value[0]
        /// int    digitalRead(uint8_t pin);
        uint8_t u8_pin;

        if (checkLength(1, pCharacteristic)) {
            u8_pin = static_cast<uint8_t>(value[0]);
            // Although ``digitialRead`` returns an ``int``, store it in a ``char``, since we assume it's a one-bit value.
            ret[0] = static_cast<char>(digitalRead(u8_pin));
            snprintf(buf, sizeof(buf), "%u = digitalRead(%u)\n", ret[0], u8_pin);
            ret.replace(1, 99, buf);
        }
    };
};


// Same as above, for ``ledcWrite``.
class LedcWriteCallback: public InvokeArduinoCallback {
    void onWrite(BLECharacteristic* pCharacteristic) {
        // This wraps:
        ///                    value[0]      value[4:1]
        /// void ledcWrite(uint8_t channel, uint32_t duty)
        uint8_t u8_channel;
        uint32_t u32_duty;

        if (checkLength(5, pCharacteristic)) {
            // Extract function parameters.
            u8_channel = static_cast<uint8_t>(value[0]);
            // Since the data isn't aligned, use memcpy.
            memcpy(&u32_duty, value.data() + 1, 4);

            // Call the function.
            snprintf(buf, sizeof(buf), "ledcWrite(%u, 0x%X)\n", u8_channel, u32_duty);
            ret.assign(buf);
            //ledcWrite(u8_channel, u32_duty);
        }
    };
};


// Same as above, for ``ledcWrite``.
class LedcSetupCallback: public InvokeArduinoCallback {
    void onWrite(BLECharacteristic* pCharacteristic) {
        // This wraps:
        ///                       value[0]    value[9:1]         value[10]
        /// double ledcSetup(uint8_t channel, double freq, uint8_t resolution_bits)
        uint8_t u8_channel;
        double d_freq;
        uint8_t u8_resolution_bits;
        double d_ret;

        if (checkLength(11, pCharacteristic)) {
            // Extract function parameters.
            u8_channel = static_cast<uint8_t>(value[0]);
            memcpy(&d_freq, value.data() + 1, 8);
            u8_resolution_bits = static_cast<uint8_t>(value[10]);

            // Call the function.
            snprintf(buf, sizeof(buf), "ledcSetup(%u, %lf, %u)\n", u8_channel, d_freq, u8_resolution_bits);
            ret.replace(9, 91, buf);
            d_ret = ledcSetup(u8_channel, d_freq, u8_resolution_bits);
            ret.assign(static_cast<char>(d_ret), 8);
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

    pCharacteristic = pService->createCharacteristic(
        DIGITAL_READ_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristic->setCallbacks(new DigitalReadCallback());

    pCharacteristic = pService->createCharacteristic(
        LEDC_SETUP_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristic->setCallbacks(new LedcSetupCallback());

    pCharacteristic = pService->createCharacteristic(
        LEDC_WRITE_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE
    );
    pCharacteristic->setCallbacks(new LedcWriteCallback());

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
