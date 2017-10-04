#!/bin/node
const LinuxApp = require('linux-app');

const url = process.env['MAKESTACK_SERVER_URL'];
const deviceId = process.env['MAKESTACK_DEVICE_ID'];
const deviceSecret = process.env['MAKESTACK_DEVICE_SECRET'];

let app = new LinuxApp(url, deviceId, deviceSecret);
app.run();
