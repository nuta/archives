# Dos and Don'ts

## The server-side part is a serverless function
:x:
```js
let uptime = 0
// Don't run the program forever!
setInterval(() => {
    uptime++;
}, 1000);

app.post("/update", (req, res) => {
    res.send(uptime)
})
```

## The device and the server **cannot** share any variables
:x:
```js
const message = "Hello"
app.post("/update", (req, res) => {
    message = req.body
})

app.onReady((device) => {
    // This causes a runtime error because we cannot use the `message` variable
    // here. All what we have is only `device` variable.
    print(message)
})
```

:white_check_mark: To send a value to the service from the device, use `publish` API:
```js
app.onEvent("my-sensor-data", (value) => {
    console.log("received a data from a device:", value);
})

app.onReady((device) => {
    while (1) {
        const data = analogRead(10);
        device.publish("my-sensor-data", data);
        device.delaySeconds(15);
    }
})
```

:warning: Sending a value to the device from the server **is not yet implemented**.

## The device API is not available in server contexts
:x:
```js
app.onReady((device) => {
    device.pinMode(11, "OUTPUT");
    // The device and server does NOT share the global scope! This simply causes
    // a undefined varible reference (`app`) error.
    app.post("/turn_on", (value) => {
        device.digitalWrite(11, true);
    })
})
```


## `console.log` is not avialable in device cotexts
:x:
```js
app.onReady((device) => {
    while (1) {
        // This causes a runtime error because `console` is not available in a
        // device context.
        console.log("Hello!");

        device.delaySeconds(15);
    }
})
```

:white_check_mark: Use `print` API instead:
```js
app.onReady((device) => {
    while (1) {
        device.print("Hello!");
        device.delaySeconds(15);
    }
})
```
