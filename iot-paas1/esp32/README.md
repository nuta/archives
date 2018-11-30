MakeStack OS for ESP32
======================

MakeStack OS for ESP32, a firmware for MakeStack apps on [ESP32](https://espressif.com/en/products/hardware/esp32/overview).

Building
--------
```
$ make -j8
```

Flashing
--------
```
$ make flash
```

Components
----------
MakeStack OS for ESP32 consists of these components:

- **[ESP-IDF](https://github.com/espressif/esp-idf):** The official development framework.
- **[JerryScript](http://jerryscript.net/):** A lightweight JavaScript engine.
- **Supervisor:** A process in the OS responsibles for communicating with MakeStack Server and managing an MakeStack app.
- **Plugins:** A plugin packages for ESP32 apps including MakeStack API (`@makestack`). (in `esp32/plugins`)
