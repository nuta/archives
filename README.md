esptoolpy
=========

A Node.js package that bundles [esptool](https://github.com/espressif/esptool).

```js
const esptoolpy = require('esptoolpy')

// Returns ChildProcess
const cp = esptoolpy([
    '--chip', 'esp32',
    '--port', '/dev/tty.usbserial-142430',
    '--baud', '115200',
    '--before', 'default_reset',
    '--after', 'hard_reset',
    'write_flash',
    '-z',
    '--flash_mode', 'dio', '--flash_freq', '40m', '--flash_size', 'detect',
    '0xe000', ' partitions/boot_app0.bin',
    '0x1000', ' bootloader/bootloader.bin',
    '0x10000', 'firmware.bin',
    '0x8000', ' default.bin'
])

cp.stdout.on('data' s => console.log(s))
cp.stderr.on('data' s => console.error(s))
```
