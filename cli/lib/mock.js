let fs = require("fs");
let path = require("path");
let fetch = require("node-fetch");
let LinuxApp = require("linux-app");
let api = require("./api");
let config = require("./config");

function create(args, opts, logger) {
    let deviceName = args.name;
    api.registerDevice(deviceName, "mock", null).then(r => {
        Object.assign(config.mocks, { deviceName: r.json });
    }).catch(e => {
        logger.error("failed to create a mock device", e);
    });
}

function run(args, opts, logger) {
    let mock = config.mocks[args.name];
    (new LinuxApp(config.server.url, mock.device_id, mock.device_secret).run();
}

module.exports = { create, run };
