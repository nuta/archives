#!/bin/node
const AppRuntime = require('app-runtime');

const url = process.env['MAKESTACK_SERVER_URL'];
const deviceId = process.env['MAKESTACK_DEVICE_ID'];
const deviceSecret = process.env['MAKESTACK_DEVICE_SECRET'];

console.log("*** starting agent");
let app = new AppRuntime(url, deviceId, deviceSecret);
app.run();
