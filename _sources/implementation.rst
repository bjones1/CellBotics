**************
Implementation
**************
This section of the book presents the core code which implements the CellBot's ability to connect a smart device to the mobile base. It consists of two programs:

-   JavaScript code which runs in a web browser and send commands to the ESP32 microcontroller on the mobile base.
-   Arduino/C++ code which runs on the ESP32 mobile base which receives commands from the web browser, executes them, then returns the results back to the browser.

It also contains file used to create this book.


Programs
========
.. toctree::
    :maxdepth: 2

    Arduino/BLE_server/BLE_server.ino
    web/toctree


Documentation support
=====================
.. toctree::
    :maxdepth: 2

    docs/.gitignore
    docs/conf.py
    docs/pavement.py
    docs/codechat_config.json
