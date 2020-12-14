// *************************************************************
// |docname| - JavaScript code to connect with a CellBot via BLE
// *************************************************************

"use strict";


class CellBotBle {
    constructor() {
        this.clear_connection();
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

    }

    // Generic access function
    async invoke_Arduino(characteristic) {
        // Get function args[1:].
        let args = Array.prototype.slice.call(arguments, 1);
        await characteristic.writeValue(new Uint8Array(args));
        // Read the return value and return it.
        let val = await characteristic.readValue();
        return String.fromCharCode.apply(null, new Uint8Array(val.buffer));
    }

    // Invoke `pinMode <https://www.arduino.cc/reference/en/language/functions/digital-io/pinmode/>`_ on the Arduino.
    async pinMode(pin, mode) {
        return this.invoke_Arduino(this.pinMode_char, pin, mode);
    }

    // Invoke `digitalWrite <https://www.arduino.cc/reference/en/language/functions/digital-io/digitalwrite//>`_ on the Arduino.
    async digitalWrite(pin, value) {
        return this.invoke_Arduino(this.digitalWrite_char, pin, value);
    }
}


// Provide a simple pair/disconnect GUI for the CellBot Bluetooth connection.
class CellBotBleGui {
    constructor() {
        this.LED1 = 2;
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

            console.log(await this.cell_bot_ble.digitalWrite(this.LED1, 1));
        } else {
            // TODO: we make the assumption that the text of the button (pair or disconnect) is up to date. Is this always correct?
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
