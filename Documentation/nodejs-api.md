Node.js API
============

## Global
There functions are definied as global one. You don't have to `require()` to use.

### plugin
- **Definition:** `(name: string) => any`
- **Description:** A `require()` wrapper function to load the specified plugin.
- **Usage:**
  ```js
  const HDC1000 = plugin('HDC1000')
  const TemperatureSensor = new HDC1000()
  ```

## Logging
You don't have to `require` to use. This API is defined as a global `Logging` object.

### print
- **Definition:** `(message: string) => void`
- **Usage:**
  ```js
  Logging.print("Hello World!")
  ```

### error
- **Definition:** `(message: string) => void`
- **Usage:**
  ```js
  Logging.error("Something went wrong :(")
  ```

## Store
You don't have to `require` to use. This API is defined as a global `Store` object.

### onCommand
- **Definition:** `(key: string, callback: (newValue: string) => void) => void`
- **Usage:**
  ```js
  const pumpRelay = new GPIO({ pin: 13, mode: GPIO.OUTPUT })
  Store.onCommand('water-the-plant', duration => {
    pumpRelay.write(true)
    setTimeout(pumpRelay.write(false), parseInt(duration))
  })
  ```

### onChange
- **Definition:** `(key: string, callback: (newValue: string) => void) => void`
- **Usage:**
  ```js
  const AQM0802A = plugin('aqm0802a')
  const display = new AQM0802A()
  Store.onCHange('displayMessage', dislayMessage => {
    display.update(displayMessage)
  })
  ```

## Event
You don't have to `require` to use. This API is defined as a global `Event` object.

### publish
- **Definition:**
  -  `(event: string, data: string | number) => void`
  -  `(event: string) => void`
- **Usage:**
  ```js
  Event.publish('temperature', 25)
  ```

## Timer
You don't have to `require` to use. This API is defined as a global `Timer` object.

### loop
- **Definition:** `(callback: () => void) => void`
- **Warning:** Use `Timer.sleep()` in the callback or it becomes a busy loop.
- **Usage:**
  ```js
  const led = new GPIO({ pin: 13, mode: GPIO.OUTPUT })
  Timer.loop(async () => {
    led.write(true)
    await Timer.sleep(1)
    led.write(false)
    await Timer.sleep(0.5)
  })
  ```

### interval
- **Definition:** `(interval: number /* seconds */, callback: () => void) => void`
- **Usage:**
  ```js
  const HDC1000 = plugin('hdc1000')
  const sensor = new HDC1000()
  Timer.interval(15, async () => {
    Event.publish("temperature", sensor.readTemperature())
  })
  ```

## Subprocess
You don't have to `require` to use. This API is defined as a global `SubProcess` object.

**Definitions:**
```typescript
interface SubProcessResult {
  stdout: string;
  stderr: string;
  status: number;
}
```

### run
- **Definition:** `(argv: string[]) => SubProcessResult`
- **Description:** Spawns a child process and blocks until it exits.
- **Usage:**
  ```js
  const { stdout, stderr, status } = SubProcess.run(['./app-written-in-golang'])
  if (status != 0) {
    Logging.error(`child process returned ${status}:\n${stderr}`)
  }
  ```

## GPIO
You don't have to `require` to use. This API is defined as a global `GPIO` object.

### INPUT
- **Definition:** `string`
- **Usage:**
  ```js
  GPIO.INPUT
  ```

### OUTPUT
- **Definition:** `string`
- **Usage:**
  ```js
  GPIO.OUTPUT
  ```

### setMode
- **Definition:** `(mode: GPIO.INPUT | GPIO.OUTPUT) => void`
- **Usage:**
  ```js
  const led = new GPIO({ pin: 13, mode: GPIO.OUTPUT })
  led.setMode(GPIO.INTPUT)
  ```

### write
- **Definition:** `(value: boolean) => void`
- **Usage:**
  ```js
  const led = new GPIO({ pin: 13, mode: GPIO.OUTPUT })
  led.write(true)
  ```

### read
- **Definition:** `() => boolean`
- **Usage:**
  ```js
  const button = new GPIO({ pin: 13, mode: GPIO.INPUT })
  Timer.interval(0.5, () => {
    const value = button.read()
    Logging.print(`button: ${value}`)
  })
  ```

### onInterrupt
- **Definition:**
  - `(mode: 'rising' | 'falling' | 'both', callback: () => void) => void`
  - `(/* mode = 'rising' */ callback: () => void) => void`
- **Usage:**
  ```js
  const button = new GPIO({ pin: 13, mode: GPIO.INPUT })
  button.onInterrupt('rising', () => {
    Event.publish('button-pressed')
  })
  ```

### onChange
- **Definition:** `(callback: () => void)`
- **Usage:**
  ```js
  const button = new GPIO({ pin: 13, mode: GPIO.INPUT })
  button.onChange(() => {
    Event.publish('button', button.read())
  })
  ```

## I2C
You don't have to `require` to use. This API is defined as a global `I2C` class.

### Constructor
- **Definition:** `({ address: number }) => I2C`
- **Usage:**
  ```js
  const device = new I2C({ address: 0x40 })
  ```

### read
- **Definition:** `(length: number) => Buffer`
- **Usage:**
  ```js
  const data = device.read(2)
  ```

### write
- **Definition:** `(data: Buffer) => void`
- **Usage:**
  ```js
  device.write([0x01, 0x00, 0x00])
  ```

## Serial
**CAUTION: Serial API may not work. Its implementation is still buggy.**

You don't have to `require` to use. This API is defined as a global `Serial` class.

### Constructor
- **Definition:** `({ path: string, baudrate: 9600 | 115200 }) => I2C`
- **Usage:**
  ```js
  const port = new Serial({ path: '/dev/cu.usbmodem1421', baudrate: 115200 })
  ```

### Serial.list
- **Definition:** `() => [string]`
- **Description:** Returns a list of serial devices.
- **Usage:**
  ```js
  Serial.list()
  ```

### read
- **Definition:** `() => Buffer`
- **Usage:**
  ```js
  port.read()
  ```

### write
- **Definition:** `(data: Buffer) => void`
- **Usage:**
  ```js
  port.write('Hello!\n')
  ```

### onData
- **Definition:** `(callback: (data) => void)`
- **Usage:**
  ```js
  port.onData(chunk => {
    console.log(chunk)
  })
  ```

### onNewLine
- **Definition:** `(callback: (line) => void)`
- **Description:** Polls data from the serial port and call `callback` each line.  The trailing `\r` and `\n` are removed.
- **Usage:**
  ```js
  port.onLine(line => {
    console.log(line)
  })
  ```

## SPI
**CAUTION: SPI API may not work. Its implementation is still buggy.**

You don't have to `require` to use. This API is defined as a global `SPI` class.

### Constructor
- **Definition:** `({ slave: number, speed: number, mode: 'MODE0' | 'MODE1' | 'MODE2' | 'MODE3' }) => SPI`
- **Usage:**
  ```js
  const device = new SPI({ slave: 0 /* depends on the device */, speed: 100000, mode: 'MODE3' })
  ```

### transfer
- **Definition:** `(tx: Buffer) => Buffer`
- **Usage:**
  ```js
  console.log(device.transfer([0x30, 0x00, 0x00]))
  ```
