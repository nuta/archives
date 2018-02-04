# esptool.js (Work-in-Progress)

Yet another [esptool](https://github.com/espressif/esptool) implementation in JavaScript,
for Node.js and web browsers ([WebUSB](https://wicg.github.io/webusb/)).

## ToDo
- [x] Support USB-to-UART device used by ESP32-DevKitC (CP2102)
- [x] Implement `SYNC`
- [ ] Implement flashing **(WIP)**
- [ ] reset-to-bootloader
- [ ] Publish online demo
- [ ] Upload bootloader stub
- [ ] Support compressed flash uploading
- [ ] [WebSerial API](http://whatwg.github.io/serial/)

## References
- **[Serial Protocol](https://github.com/espressif/esptool/wiki/Serial-Protocol):** somewhat outdated
- **[esptool.py Source Code](https://github.com/espressif/esptool/blob/master/esptool.py):** The best reference.
- **[esptoo.py trace](https://gist.github.com/seiyanuta/1674f2a1c8d01c72add5f1846d14073a)**

## License
Public Domain or MIT **excluding** `*.bin` files. Choose whichever you prefer.
