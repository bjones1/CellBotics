// *************************************************************
// |docname| - JavaScript code to connect with a CellBot via BLE
// *************************************************************

"use strict";


class CellBotBle {
    constructor() {
        this.clear_connection();
        // If true, the server (BLE device / CellBot) is little-endiang; if false, big-endian.
        this.is_little_endian = true;
    }


    // Clear all the Bluetooth connection-related objects.
    clear_connection() {
        this.server = undefined;
        this.pinMode_char = undefined;
        this.digitalWrite_char = undefined;
    }

    // Returns true if the Bluetooth device (server) is connected.
    paired() {
        return this.server && this.server.connected;
    }

    // Pair with a CellBot and return the characteristic used to control the device.
    async pair(disconnect_callback)
    {
        // Request a device with service `UUIDs`. See the `Bluetooth API <https://developer.mozilla.org/en-US/docs/Web/API/Bluetooth>`_.
        let cellBot_service = "6c533793-9bd6-47d6-8d3b-c10a704b6b97";

        // Skip connecting if we're already connected.
        if (!this.paired()) {
            this.device = await navigator.bluetooth.requestDevice({
                filters: [{
                    services: [cellBot_service]
                }]
            });

            // Notify on a disconnect. I can't find any docs on this, but it does work.
            this.device.addEventListener('gattserverdisconnected', disconnect_callback);

            // Connect to its server.
            this.server = await this.device.gatt.connect();
        }

        // Get the service for our server.
        let service = await this.server.getPrimaryService(cellBot_service);

        // Get the characteristic for this service.
        this.pinMode_char = await service.getCharacteristic("6ea6d9b6-7b7e-451c-ab45-221298e43562");
        this.digitalWrite_char = await service.getCharacteristic("d3423cf6-6da7-4dd8-a5ba-3c980c74bd6d");
        this.digitalRead_char = await service.getCharacteristic("c370bc79-11c1-4530-9f69-ab9d961aa497");
        this.ledcSetup_char = await service.getCharacteristic("6be57cea-3c46-4687-972b-03429d2acf9b");
        this.ledcAttachPin_char = await service.getCharacteristic("2cd63861-078f-436f-9ed9-79e57ec8b638");
        this.ledcWrite_char = await service.getCharacteristic("40698030-a343-448f-a9ea-54b39b03bf81");
    }

    // Generic access function for calling a function on the Arduino. It returns (value returned after invoking the function, message).
    async invoke_Arduino(
        // The Bluetooth characteristic to use for this call.
        characteristic,
        // The number of bytes in the return value:
        //
        // -    0: void
        // -    +1/-1: unsigned/signed 8-bit value
        // -    +2/-2: unsigned/signed 16-bit value
        // -    +4/-4: unsigned/signed 32-bit value
        // -    0.4/0.8: 32-bit/64-bit float
        return_bytes,
        // An ArrayBuffer or compatible type of data containing encoded parameters to send.
        param_array
    ) {
        await characteristic.writeValue(param_array);
        // Read the returned data.
        let return_data = await characteristic.readValue();
        // Interpret the return value.
        let return_value;
        switch (return_bytes) {
            case 0:
            return_value = undefined;
            break;

            case 1:
            return_value = return_data.getUint8(0);
            break;

            case -1:
            return_value = return_data.getInt8(0);
            break;

            case 2:
            return_value = return_data.getUint16(0);
            break;

            case -2:
            return_value = return_data.getInt16(0, this.is_little_endian);
            break;

            case 4:
            return_value = return_data.getUint32(0, this.is_little_endian);
            break;

            case -4:
            return_value = return_data.getInt32(0, this.is_little_endian);
            break;

            case 0.4:
            return_value = return_data.getFloat32(0, this.is_little_endian);
            return_bytes = 4;
            break;

            case 0.8:
            return_value = return_data.getFloat64(0, this.is_little_endian);
            return_bytes = 8;
            break;

        }

        let message = return_data.buffer.slice(return_bytes);
        return [return_value, String.fromCharCode.apply(null, new Uint8Array(message))];
    }

    // Invoke `pinMode <https://www.arduino.cc/reference/en/language/functions/digital-io/pinmode/>`_ on the Arduino.
    async pinMode(u8_pin, u8_mode) {
        return this.invoke_Arduino(this.pinMode_char, 0, new Uint8Array([u8_pin, u8_mode]));
    }

    // Invoke `digitalWrite <https://www.arduino.cc/reference/en/language/functions/digital-io/digitalwrite/>`_ on the Arduino.
    async digitalWrite(u8_pin, u8_value) {
        return this.invoke_Arduino(this.digitalWrite_char, 0, new Uint8Array([u8_pin, u8_value]));
    }

    // Invoke `digitalRead <https://www.arduino.cc/reference/en/language/functions/digital-io/digitalread/>`_ on the Arduino.
    async digitalRead(u8_pin) {
        return this.invoke_Arduino(this.digitalRead_char, 1, new Uint8Array([u8_pin]));
    }

    // Invoke ``ledcSetup`` on the Arduino.
    //
    // Note that the LEDC control on the ESP32 Arduino port isn't documented. Here's my attempts. The best reference is the `LED_PWM chapter of the ESP32 Technical Reference Manual <https://www.espressif.com/sites/default/files/documentation/esp32_technical_reference_manual_en.pdf#page=384>`_. To set up PWM, you need to select:
    //
    // -    A channel (channels 0-7 auto-update new PWM periods, channels 8-15 don't).
    // -    The frequency to do the PWM -- either 80 MHz or 1 MHz.
    // -    A number of bits used to do the PWM. The maximum possible value is floor(log2(processor clock frequency/PWM frequency)); this cannot exceed 20.
    //
    // The function returns the actual PWM frequency, due to the limitations of the available clock divisor.
    async ledcSetup(u8_channel, d_freq, u8_resolution_bits) {
        let param_array = new ArrayBuffer(11);
        let dv = new DataView(param_array);
        dv.setUint8(0, u8_channel);
        dv.setFloat64(1, d_freq, this.is_little_endian);
        dv.setUint8(10, u8_resolution_bits);
        return this.invoke_Arduino(this.ledcSetup_char, 0.8, param_array);
    }

    // Invoke ``ledcAttachPin`` on the Arduino.
    //
    // Next, attach this channel to a specific pin on the Arduino.
    async ledcAttachPin(u8_pin, u8_channel) {
        return this.invoke_Arduino(this.ledcAttachPin_char, 0, new Uint8Array([u8_pin, u8_channel]));
    }

    // Invoke ``ledcWrite`` on the Arduino.
    //
    // Finally, select a duty cycle for that channel, from 2^num_bits to 1.
    async ledcWrite(u8_channel, u32_duty) {
        let param_array = new ArrayBuffer(5);
        let dv = new DataView(param_array);
        dv.setUint8(0, u8_channel);
        dv.setUint32(1, u32_duty, this.is_little_endian);
        return this.invoke_Arduino(this.ledcWrite_char, 0, param_array);
    }
}


// Provide a simple pair/disconnect GUI for the CellBot Bluetooth connection.
class CellBotBleGui {
    constructor() {
        this.PB1 = 0;
        this.LED1 = 2;

        this.INPUT = 1;
        this.OUTPUT = 2;

        this.ble_pair_button = document.getElementById("ble_pair_button");
        this.ble_pair_status = document.getElementById("ble_pair_status");

        this.cell_bot_ble = new CellBotBle();
    }

    async async_on_pair_clicked() {
        if (!this.cell_bot_ble.paired()) {
            this.ble_pair_button.disabled = true;
            this.ble_pair_status.innerHTML = "Pairing...";
            try {
                await this.cell_bot_ble.pair(this.on_disconnect_bound());
                this.ble_pair_status.innerHTML = `Paired to ${this.cell_bot_ble.device.name}.`;
                this.ble_pair_button.innerHTML = "Disconnect";

            } catch (err) {
                this.ble_pair_status.innerHTML = "Unable to pair.";
                throw err;
            } finally {
                this.ble_pair_button.disabled = false;
            }

            console.log(await this.cell_bot_ble.pinMode(this.LED1, this.OUTPUT));
            console.log(await this.cell_bot_ble.pinMode(this.PB1, this.INPUT));
            console.log(await this.cell_bot_ble.digitalWrite(this.LED1, 1));
            console.log(await this.cell_bot_ble.digitalRead(this.PB1));
            console.log(await this.cell_bot_ble.ledcSetup(0, 1000, 16));
            console.log(await this.cell_bot_ble.ledcAttachPin(this.LED1, 0));
            console.log(await this.cell_bot_ble.ledcWrite(0, 32767));
        } else {
            this.cell_bot_ble.server.disconnect();
        }
    }

    // A reference to a class method is just a function, not a bound method. (Crazy!) Therefore, ``this`` when invoked from the callback won't refer to this class. (Really crazy.) So, create our own bound method by return a function with a ``this`` actually bound to this class. This means that ``this.ble_pair_status`` and similar references will now work.
    on_disconnect_bound() {
        return function() {
            this.ble_pair_status.innerHTML = "Disconnected.";
            this.ble_pair_button.innerHTML = "Pair";
        }.bind(this);
    }
}


let cell_bot_ble_gui;

function on_dom_ready() {
    cell_bot_ble_gui = new CellBotBleGui();
}


// This is invoked after the DOM is ready.
(function() {
    on_dom_ready();
})();


// Pair with the CellBot.
function ble_pair()
{
    cell_bot_ble_gui.async_on_pair_clicked().then();
}
