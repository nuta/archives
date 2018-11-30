# Server API

## How to use it
```js
const { server, Device } = require("makestack")
```

## Device
### Device.getByName()
- **Definition:** `(name: string): Device`

### .data
- **Definition:** `{ [key: string]: any }`

### .save()
- **Definition:** `(): void`

### .command()
- **Definition:** `(command: string, arg: string): void`

## server
### server.http.get()
- **Definition:** `(path: string, (req: express.Request, res: express.Response) => void): void`
- **Example:**
    ```js
    const { server, Device } = require("makestack")

    server.http.get("/light/turn_on", (req, res) => {
        const device = Device.getByName(req.params.device);
        device.command("turn_on", "light");
        device.save();

        res.send("Enqueued the command.");
    })
    ```

### server.device.onHeartbeat()
- **Definition:** `((device: Device) => void): void`
- **Example:**
    ```js
    const { server, Device } = require("makestack")

    server.device.onHeartbeat((device) => {
        device.command("display", getLatestForecastFromSomewhere());
        // device.save() will automatically executed.
    })
    ```

### server.device.onBoot()
- **Definition:** `((device: Device) => void): void`
- **Example:**
    ```js
    const { server, Device } = require("makestack")

    server.device.onBoot((device) => {
        device.data.heartbeats_num = 0;
        // device.save() will automatically executed.
    })

    server.device.onHeartbeat((device) => {
        device.data.heartbeats_num++;
        // device.save() will automatically executed.
    })
    ```

### server.device.onEvent()
- **Definition:** `(event: string, (device: Device, value: any) => void): void`
- **Example:**
    ```js
    const { server, Device } = require("makestack")

    server.device.onEvent("button", (device, eventName, value) => {
        if (value === "pressed")
            postToSlack(`${device.name}: pressed!`);
    })
    ```
