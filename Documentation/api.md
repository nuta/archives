API
====

Your MakeStack app is executed on [Node.js](https://nodejs.org) version 8.9.0. You
can use [all modules bundled in Node.js](https://nodejs.org/api/).

## Using plugin
Plugins provide a Node.js package such as device drivers. Currenly MakeStack does
not support npm packages and it ignores `package.json`.

### Using a official plugin
To use a plugin add its name to `app.yaml`:

```yaml
# app.yaml
name: my-app
plugins: ['hdc1000']
```

### Using a third-party plugin hosted on GitHub
To use a plugin on GitHub add its name with `<github-username>/` prefix to `app.yaml`:

```yaml
# app.yaml
name: my-app
plugins: ['seiyanuta/hdc1000']
```

### Loading a plugin
To load a plugin use `require()` with the `@makestack/` prefix:
```js
const { HDC1000 } = require('@makestack/HDC1000')
const TemperatureSensor = new HDC1000()
```

## Globals
You don't have to `require` to use. These functions are defined as a global ones.

### println
- **Definition:** `(message: string) => void`
- **Usage:**
  ```js
  println("Hello World!")
  ```

### error
- **Definition:** `(message: string) => void`
- **Usage:**
  ```js
  error("Something went wrong :(")
  ```

### publish
- **Definition:**
  -  `(event: string, data: string | number) => void`
  -  `(event: string) => void`
- **Usage:**
  ```js
  publish('temperature', 25)
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
    publish("temperature", sensor.readTemperature())
  })
  ```

### sleep
- **Definition:** `async (duration: number /* seconds */) => void`
- **Usage:**
  ```js
  const led = new GPIO({ pin: 13, mode: GPIO.OUTPUT })
  Timer.loop(async () => {
    led.write(true)
    await Timer.sleep(0.3)
    led.write(false)
    await Timer.sleep(0.3)
  })
  ```

### busywait
- **Definition:** `(usec: number /* microseconds */) => void`
- **Warning:** As its name implies it halts the app event loop. Avoid to use this API.
- **Usage:**
  ```js
  const led = new GPIO({ pin: 13, mode: GPIO.OUTPUT })
  Timer.interval(1, () => {
    led.write(true)
    Timer.busywait(1000)
    led.write(false)
    Timer.busywait(1000)
  })
  ```

## App
You don't have to `require` to use. This API is defined as a global `App` object.

### onExit
- **Definition:** `(callback: () => void): void`
- **Description:** Enable OS/app update (enabled by default).
- **Usage:**
  ```js
  App.onExit(() => {
    console.log("I'll be back.")
  })
  ```

### enableUpdate
- **Definition:** `(): void`
- **Description:** Enable OS/app update (enabled by default).

### disableUpdate
- **Definition:** `(): void`
- **Description:** Disable OS/app update.
- **Warning:** Don't forget to re-enable update!
- **Usage:**
  ```js
  Timer.interval(3 () => {
    App.disableUpdate()
    try {
      // Do important stuff.
    } finally {
      App.enableUpdate()
    }
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
    error(`child process returned ${status}:\n${stderr}`)
  }
  ```

## GPIO
You don't have to `require` to use. This API is defined as a global `GPIO` object.

## Constructor
- **Definition:** `({ pin: number, mode: GPIO.INPUT | GPIO.OUTPUT })`
- **Usage:**
  ```js
  const led = new GPIO({ pin: 13, mode: GPIO.OUTPUT })
  ```

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
    println(`button: ${value}`)
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
    publish('button-pressed')
  })
  ```

### onChange
- **Definition:** `(callback: () => void)`
- **Usage:**
  ```js
  const button = new GPIO({ pin: 13, mode: GPIO.INPUT })
  button.onChange(() => {
    publish('button', button.read())
  })
  ```

## I2C
You don't have to `require` to use. This API is defined as a global `I2C` class.

### Constructor
- **Definition:** `({ address: number })`
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
- **Definition:** `(data: number[] | Buffer) => void`
- **Usage:**
  ```js
  device.write([0x01, 0x00, 0x00])
  ```

## Serial
**CAUTION: Serial API may not work. Its implementation is still buggy.**

You don't have to `require` to use. This API is defined as a global `Serial` class.

### Constructor
- **Definition:** `({ path: string, baudrate: 9600 | 115200 })`
- **Usage:**
  ```js
  const port = new Serial({ path: '/dev/cu.usbmodem1421', baudrate: 115200 })
  ```ccccc

### Serial.list
- **Definition:** `() => [string]`
- **Description:** Returns a list of serial devices.
- **Usage:**
  ```js
  Serial.list() //=> ['/dev/ttyUSB0']
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
- **Definition:** `({ slave: number, speed: number, mode: 'MODE0' | 'MODE1' | 'MODE2' | 'MODE3' })`
- **Usage:**
  ```js
  const device = new SPI({ slave: 0 /* depends on the device */, speed: 100000, mode: 'MODE3' })
  ```

### transfer
- **Definition:** `(tx: number[] | Buffer) => Buffer`
- **Usage:**
  ```js
  console.log(device.transfer([0x30, 0x00, 0x00]))
  ```
