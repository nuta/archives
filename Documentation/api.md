# API

## Runtime environment
Your MakeStack app is executed on [Node.js](https://nodejs.org) version 8.9.0. You
can use [all modules bundled in Node.js](https://nodejs.org/api/).

## Using npm dependencies
To bundle npm pacakges into the app, use [yarn](https://yarnpkg.com/). MakeStack detects `yarn.lock` and build & install npm packages automatically. In a nutshell:

```bash
yarn init
yarn add express lodash package-what-you-want
```

You have to deploy the app from `makestack deploy` command. **Desktop and Web does not support npm dependencies.**

## Using plugin
Plugins is a npm package such as I2C device drivers. Official plugins are bundled in the OS so you don't
have to install them. To use a official plugin use `require()` with the `@makestack/` prefix:

```js
const { HDC1000 } = require('@makestack/hdc1000')
const TemperatureSensor = new HDC1000()
```

Available plugins are listed on: https://github.com/makestack/makestack/tree/master/plugins

## Using APIs
```js
const { print, GPIO, I2C } = require('makestack')
print('Hello, World!')
```

## Functions
### How to use
```js
const { print, error, publish } = require('makestack')
```

### print
- **Definition:** `(message: string) => void`
- **Example:**
  ```js
  print("Hello World!")
  ```

### error
- **Definition:** `(message: string) => void`
- **Example:**
  ```js
  error("Something went wrong :(")
  ```

### publish
- **Definition:**
  -  `(event: string, data: string | number) => void`
  -  `(event: string) => void`
- **Example:**
  ```js
  publish('temperature', 25)
  ```

## Config
### How to use
```js
const { Config } = require('makestack')
```

### onCommand
- **Definition:** `(key: string, callback: (newValue: string) => void) => void`
- **Example:**
  ```js
  const pumpRelay = new GPIO({ pin: 13, mode: 'out' })
  Config.onCommand('water-the-plant', duration => {
    pumpRelay.write(true)
    setTimeout(pumpRelay.write(false), parseInt(duration))
  })
  ```

### onChange
- **Definition:** `(key: string, callback: (newValue: string) => void) => void`
- **Example:**
  ```js
  const AQM0802A = plugin('aqm0802a')
  const display = new AQM0802A()
  Config.onCHange('displayMessage', dislayMessage => {
    display.update(displayMessage)
  })
  ```

## Timer
### How to use
```js
const { Timer } = require('makestack')
```

### interval
- **Definition:** `(interval: number /* seconds */, callback: () => void) => void`
- **Example:**
  ```js
  const HDC1000 = plugin('hdc1000')
  const sensor = new HDC1000()
  Timer.interval(15, async () => {
    publish("temperature", sensor.readTemperature())
  })
  ```

### sleep
- **Definition:** `async (duration: number /* seconds */) => void`
- **Example:**
  ```js
  const led = new GPIO({ pin: 13, mode: 'out' })
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
- **Example:**
  ```js
  const led = new GPIO({ pin: 13, mode: 'out' })
  Timer.interval(1, () => {
    led.write(true)
    Timer.busywait(1000)
    led.write(false)
    Timer.busywait(1000)
  })
  ```

## App
### How to use
```js
const { App } = require('makestack')
```

### onExit
- **Definition:** `(callback: () => void): void`
- **Description:** Enable OS/app update (enabled by default).
- **Example:**
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
- **Example:**
  ```js
  Timer.interval(3, () => {
    App.disableUpdate()
    try {
      // Do important stuff.
    } finally {
      App.enableUpdate()
    }
  })
  ```

## Subprocess
### How to use
```js
const { SubProcess } = require('makestack')
```

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
- **Example:**
  ```js
  const { stdout, stderr, status } = SubProcess.run(['./app-written-in-golang'])
  if (status != 0) {
    error(`child process returned ${status}:\n${stderr}`)
  }
  ```

## GPIO
The pin number depends on the device.

### How to use
```js
const { GPIO } = require('makestack')
```

## Constructor
- **Definition:** `({ pin: number, mode: 'in' | 'out' })`
- **Example:**
  ```js
  const led = new GPIO({ pin: 13, mode: 'out' })
  ```

### setMode
- **Definition:** `(mode: 'in' | 'out') => void`
- **Example:**
  ```js
  const led = new GPIO({ pin: 13, mode: 'out' })
  led.setMode(GPIO.INTPUT)
  ```

### write
- **Definition:** `(value: boolean) => void`
- **Example:**
  ```js
  const led = new GPIO({ pin: 13, mode: 'out' })
  led.write(true)
  ```

### read
- **Definition:** `() => boolean`
- **Example:**
  ```js
  const button = new GPIO({ pin: 13, mode: 'in' })
  Timer.interval(0.5, () => {
    const value = button.read()
    print(`button: ${value}`)
  })
  ```

### onInterrupt
- **Definition:**
  - `(mode: 'rising' | 'falling' | 'both', callback: () => void) => void`
- **Example:**
  ```js
  const button = new GPIO({ pin: 13, mode: 'in' })
  button.onInterrupt('rising', () => {
    publish('button-pressed')
  })
  ```

### onChange
- **Definition:** `(callback: () => void)`
- **Example:**
  ```js
  const button = new GPIO({ pin: 13, mode: 'in' })
  button.onChange(() => {
    publish('button', button.read())
  })
  ```

## I2C
### How to use
```js
const { I2C } = require('makestack')
```

### Constructor
- **Definition:** `({ address: number })`
- **Example:**
  ```js
  const device = new I2C({ address: 0x40 })
  ```

### read
- **Definition:** `(length: number) => Buffer`
- **Example:**
  ```js
  const data = device.read(2)
  ```

### write
- **Definition:** `(data: number[] | Buffer) => void`
- **Example:**
  ```js
  device.write([0x01, 0x00, 0x00])
  ```

## Serial
**CAUTION: Serial API may not work. Its implementation is still buggy.**

### How to use
```js
const { Serial } = require('makestack')
```

### Constructor
- **Definition:** `({ path: string, baudrate: 9600 | 115200 })`
- **Example:**
  ```js
  const port = new Serial({ path: '/dev/cu.usbmodem1421', baudrate: 115200 })
  ```ccccc

### Serial.list
- **Definition:** `() => [string]`
- **Description:** Returns a list of serial devices.
- **Example:**
  ```js
  Serial.list() //=> ['/dev/ttyUSB0']
  ```

### read
- **Definition:** `() => Buffer`
- **Example:**
  ```js
  port.read()
  ```

### write
- **Definition:** `(data: Buffer) => void`
- **Example:**
  ```js
  port.write('Hello!\n')
  ```

### onData
- **Definition:** `(callback: (data) => void)`
- **Example:**
  ```js
  port.onData(chunk => {
    console.log(chunk)
  })
  ```

### onNewLine
- **Definition:** `(callback: (line) => void)`
- **Description:** Polls data from the serial port and call `callback` each line.  The trailing `\r` and `\n` are removed.
- **Example:**
  ```js
  port.onLine(line => {
    console.log(line)
  })
  ```

## SPI
**CAUTION: SPI API may not work. Its implementation is still buggy.**

### How to use
```js
const { SPI } = require('makestack')
```

### Constructor
- **Definition:** `({ slave: number, speed: number, mode: 'MODE0' | 'MODE1' | 'MODE2' | 'MODE3' })`
- **Example:**
  ```js
  const device = new SPI({ slave: 0 /* depends on the device */, speed: 100000, mode: 'MODE3' })
  ```

### transfer
- **Definition:** `(tx: number[] | Buffer) => Buffer`
- **Example:**
  ```js
  console.log(device.transfer([0x30, 0x00, 0x00]))
  ```
