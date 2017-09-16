let fs = require("fs");
let path = require("path");
let fetch = require("node-fetch");
let LinuxApp = require("linux-app");
let api = require("./api");
let config = require("./config");

let mocksPath = path.join(config.configDir, "mocks.json");

function create(args, opts, logger) {
    let deviceName = args.name;
    api.registerDevice(deviceName, "mock", null).then(r => {
        var mocks = {};
        try {
            mocks = JSON.parse(fs.readFileSync(mocksPath));
        } catch (e) {
            // ignore
        }

        mocks[deviceName] = r.json;
        fs.writeFileSync(mocksPath, JSON.stringify(mocks));

    }).catch(e => {
        logger.error("failed to create a mock device", e);
    });
}

function parseMspPacket(packet) {
    const types = { 0x13: "app_update_request", 0x14: "app_image_url", 0x10: "store" };

    let offset = 2; // skip the header
    let appUpdateRequest = false;
    let appVersion;
    let appImageURL = "";
    let stores = {};

    while (offset < packet.length) {
        let header = packet.readUInt8(offset);

        let type = types[header & 0x7f] || "unknown";
        let extendedLength = (header & 0x80) != 0;
        let length = (extendedLength) ? packet.readUInt16BE(offset + 2) :
                                         packet.readUInt8(offset + 1);
        let dataOffset = offset + ((extendedLength) ? 4 : 2);
        let data = packet.slice(dataOffset, dataOffset + length);
        
        switch (type) {
        case "app_update_request":
            appUpdateRequest = true;
            appVersion = data.readUInt16BE(0);
            break;
        case "app_image_url":
            appImageURL = data.toString("utf-8");
            break;
        case "store":
            let keyLength = data.readUInt8(0);
            let key = data.toString("utf-8", 1, keyLength + 1);
            let value = data.toString("utf-8", 1 + keyLength);
            stores[key] = value;
            break;
        }

        offset = dataOffset + length;
    }

    return { appUpdateRequest, appVersion, appImageURL, stores };
}

function run(args, opts, logger) {
    let deviceName = args.name;
    var appVersion = 0;
    let { device_id: deviceId } = JSON.parse(fs.readFileSync(mocksPath))[deviceName];

    console.log("booting...");
    api.sendHeartbeat(deviceId, "booting", 0, "");
    let app = new LinuxApp();

    setInterval(() => {
        console.log("heartbeating...");

        api.sendHeartbeat(deviceId, "running", appVersion, app.apis.logging.getLog()).then(r => {
            r.buffer().then(buffer => {
                let r = parseMspPacket(buffer);
                let { appUpdateRequest, appImageURL, stores } = r;

                if (appUpdateRequest) {
                    console.log(`updating ${appVersion} -> ${r.appVersion}`);
                    appVersion = r.appVersion;
                    fetch(appImageURL).then(r => {
                        r.text().then(script => {
                            app.run(script);
                        });
                    });
                }

                // arduino.updateStores(stores);
            });
          });
    }, 3000);
}

module.exports = { create, run };
