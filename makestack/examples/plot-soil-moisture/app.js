const app = require("makestack")
const superagent = require("superagent")

const THINGSPEAK_API_KEY = "XXXXXXXXXXXXXXXXX"

app.onEvent("soil-moist", (moisture) => {
    console.log("Submitting a received sensor value...")
    superagent
        .get("https://api.thingspeak.com/update")
        .query({ api_key: THINGSPEAK_API_KEY })
        .query({ field1: moisture })
        .end()
})

app.onReady((device) => {
    const pin = 34
    device.print("Hello from plot-soil-moisture!")
    while (1) {
        const value = device.analogRead(pin)
        device.publish("soil-moist", value)
        device.delay(900000)
    }
})
