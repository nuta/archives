const { server } = require("makestack")

server.device.onEvent("button", (value) => {
    console.log("Pressed the button 🍦")
})
