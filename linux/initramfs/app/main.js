#!/bin/node
const AppRuntime = require('app-runtime');

const url = process.env['MAKESTACK_SERVER_URL'];
const deviceId = process.env['MAKESTACK_DEVICE_ID'];

console.log("*** starting agent");
let app = new AppRuntime(url, deviceId);
app.run();
