const { server } = require('makestack')

server.device.onEvent("hello", (device, value) => {
    console.log(`device sent ${value}`);
});
