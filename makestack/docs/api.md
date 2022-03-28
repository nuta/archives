# API Reference

## The makestack package
`makestack` creates a makestack app:
```js
const app = require("makestack")
```

## Server-side APIs
### app.get()
Registers a HTTP GET request handler.
- **Definition:** `(path: string, (req: express.Request, res: express.Response) => void): void`
- **Example:**
    ```js
    app.get("/hello", (req, res) => {
        res.send("Hello!")
    })
    ```

### app.post()
Registers a HTTP POST request handler.
- **Definition:** `(path: string, (req: express.Request, res: express.Response) => void): void`
- **Example:**
    ```js
    app.post("/hello", (req, res) => {
        res.send("Hello!")
    })
    ```

### app.put()
Registers a HTTP PUT request handler.
- **Definition:** `(path: string, (req: express.Request, res: express.Response) => void): void`
- **Example:**
    ```js
    app.put("/hello", (req, res) => {
        res.send("Hello!")
    })
    ```

### app.delete()
Registers a HTTP DELETE request handler.
- **Definition:** `(path: string, (req: express.Request, res: express.Response) => void): void`
- **Example:**
    ```js
    app.delete("/hello", (req, res) => {
        res.send("Hello!")
    })
    ```

### app.onEvent()
Registers a device event handler. A device event can be emitted from the device using `publish` API.
- **Definition:** `(eventName: string, (value: boolean | number | string) => void): void`
- **Example:**
    ```js
    app.onEvent("my-sensor-data", async (value) => {
        await slack.chat.postMessage({ channel: "sensor", text: `sensor data; ${value}` })
    })
    ```

## Device-side APIs
Device APIs are available only in *device contexts*, such as the callback of `app.onReady`.

### app.onReady()
Registers a device-side handler when the device gets ready to start the app. Just like `setup()` function in the Arduino.
- **Definition:** `((device: DeviceAPI) => void): void`
- **Example:**
    ```js
    app.onReady((device) => {
        while (1) {
            device.print("Hello World!")
            deivce.delaySeconds(3)
        }
    })
    ```

### device.print()
Prints a log message.
- **Definition:** `(message: any): void`
- **Example:**
    ```js
    device.print(`Button status: ${digitalRead(5)}`)
    ```

### device.publish()
Sends a value to the server as a device event.
- **Definition:** `(eventName: string, value: boolean | number | string): void`
- **Example:**
    ```js
    device.publish("my-sensor-data", analogRead(10))
    ```

### device.pinMode()
Sets the pin mode.
- **Definition:** `(pin: number, value: "INPUT" | "OUTPUT"): void`
- **Example:**
    ```js
    device.pinMode(12, "OUTPUT")
    ```

### device.digitalWrite()
Sets the value of a digital pin.
- **Definition:** `(pin: number, value: boolean): void`
- **Example:**
    ```js
    device.pinMode(12, "OUTPUT")
    while (1) {
        // Turn the LED on!
        device.digitalWrite(12, true)
        device.delay(100)
        // Turn the LED off!
        device.digitalWrite(12, false)
        device.delay(100)
    }
    ```

### device.digitalRead()
Reads the value of a digital pin.
- **Definition:** `(pin: number): boolean`
- **Example:**
    ```js
    device.pinMode(12, "INPUT")
    while (1) {
        const value = device.digitalRead(12)
        device.print(value ? "Button is pressed." : "Button is released .")
        device.delay(100)
    }
    ```

### device.analogRead()
Reads the value of a analog pin. The range of the value is board-dependent.
- **Definition:** `(pin: number): number`
- **Example:**
    ```js
    while (1) {
        const value = device.analogRead(15)
        device.publish("sensor-data", value)
        device.delayMinutes(15)
    }
    ```

### device.delay()
Blocks the app for the specified amount of time (milliseconds).
- **Definition:** `(milliseconds: number): void`
- **Example:**
    ```js
    device.delay(100 /* 0.1 seconds */)
    ```

### device.delaySeconds()
Blocks the app for the specified amount of time (seconds).
- **Definition:** `(seconds: number): void`
- **Example:**
    ```js
    device.delaySeconds(10 /* 10 seconds */)
    ```

### device.delayMinutes()
Blocks the app for the specified amount of time (minutes).
- **Definition:** `(minutes: number): void`
- **Example:**
    ```js
    device.delayMinutes(15 /* 15 minutes */)
    ```
