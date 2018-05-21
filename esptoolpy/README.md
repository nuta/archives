esptoolpy
=========

A Node.js package that bundles [esptool](https://github.com/espressif/esptool).

```js
const { esptool, ARDUINO_DIR } = require('esptoolpy')

// Returns ChildProcess
const cp = esptool([
    '--chip', 'esp32',
    '--port', '/dev/tty.usbserial-142430',
    '--baud', '115200',
    '--before', 'default_reset',
    '--after', 'hard_reset',
    'write_flash',
    '-z',
    '--flash_mode', 'dio', '--flash_freq', '40m', '--flash_size', 'detect',
    '0x1000', path.resolve(ARDUINO_DIR, 'tools/sdk/bootloader_dio_40m.bin'),
    '0x8000', path.resolve(ARDUINO_DIR, 'tools/partitions/default.bin'),
    '0xe000', path.resolve(ARDUINO_DIR, 'tools/partitions/boot_app0.bin'),
    '0x10000', 'firmware.bin'
])

cp.stdout.on('data' s => console.log(s))
cp.stderr.on('data' s => console.error(s))
```
