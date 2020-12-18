.. Copyright (C) 2012-2020 Bryan A. Jones.

    This file is part of CellBotics.

    CellBotics is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

    CellBotics is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along with CellBotics.  If not, see <http://www.gnu.org/licenses/>.

**************
Implementation
**************
This section of the book presents the core code which implements the CellBot's ability to connect a smart device to the mobile base. It consists of two programs:

-   JavaScript code which runs in a web browser and send commands to the ESP32 microcontroller on the mobile base.
-   Arduino/C++ code which runs on the ESP32 mobile base which receives commands from the web browser, executes them, then returns the results back to the browser.

It also contains files used to create this book.


To do
=====
-   Write code to interface with cell phone `sensors <https://developer.mozilla.org/en-US/docs/Web/API/Sensor_APIs>`_.
-   Test WebBLE, then add it to the hardware section of the book
-   Write up basic hardware connection instructions, a parts lits, etc.
-   Figure out why reprogramming is flaky -- see the `issue <https://github.com/espressif/esptool/issues/19>`_.
-   Provide an reset function, to be called when first talking to the device or as an e-stop on disconnect.
-   Provide some kind of version/CPU info, so we could talk to an older version of the code or to other types of chip.


Programs
========
.. toctree::
    :maxdepth: 2

    Arduino/BLE_server/BLE_server.ino
    web/toctree
    docs/.gitignore


Book setup and authoring
========================
.. toctree::
    :maxdepth: 2

    docs/conf.py
    docs/pavement.py
    docs/codechat_config.json
