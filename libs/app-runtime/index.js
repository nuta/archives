const os = require('os');
const { fetch, NodeVM } = require('hyperutils');
const LoggingAPI = require('./logging');
const TimerAPI = require('./timer');
const StoreAPI = require('./store');
const EventAPI = require('./event');

let GPIOAPI;
switch(os.type()) {
  case 'Linux':
    GPIOAPI = require('./linux/gpio');
    break;
  default:
    GPIOAPI = require('./mock/gpio');
    break;
}

module.exports = class {
  constructor(url, deviceId, deviceSecret) {
    this.deviceId = deviceId;
    this.deviceSecret = deviceSecret;
    this.appVersion = 0;
    this.url = url;
    this.context = {};
    this.apis = {};
    this.globals = {};

    this.registerAPI("logging", LoggingAPI);
    this.registerAPI("timer", TimerAPI);
    this.registerAPI("event", EventAPI, this.apis.logging);
    this.registerAPI("store", StoreAPI);
    this.registerAPI("gpio", GPIOAPI);
  }

  registerAPI(name, klass, ...args) {
    let api = new klass(args);
    Object.assign(this.globals, api.globals);
    this.apis[name] = api;
    return api;
  }

  resetAPIs() {
    for (let apiName in this.apis) {
      let api = this.apis[apiName].reset();
    }
  }

  parseMspPacket(packet) {
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

  sendHeartbeat(state, appVersion, log) {
    let deviceIdBuffer = Buffer.from(this.deviceId);
    let logBuffer = Buffer.from(log);
    if (logBuffer.length > 0xffff) {
      // TODO
      throw "Too long log.";
    }

    let packetLen = 8 /* header */ + 4 * 2 + 3 + 4 + deviceIdBuffer.length + logBuffer.length;
    let packet = new Buffer.alloc(packetLen);

    // Headers
    packet.writeUInt8(1, 0); // version
    packet.writeUInt8(0, 1); // reserved
    let offset = 2;

    // deviceId
    packet.writeUInt8(0x8a, offset); // type
    packet.writeUInt8(0x00, offset + 1); // reserved
    packet.writeUInt16BE(deviceIdBuffer.length, offset + 2);
    offset += 4;
    for (var i=0; i < deviceIdBuffer.length; i++) {
      packet.writeUInt8(deviceIdBuffer[i], offset);
      offset++;
    }

    // deviceInfo
    let states = { new: 1, booting: 2, ready: 3, running: 4, down: 5, reboot: 6, relaunch: 7 }
    if (!states[state]) {
      throw `Invalid device state: \`${state}'`;
    }

    packet.writeUInt8(0x0b, offset); // type
    packet.writeUInt8(0x01, offset + 1); // length
    packet.writeUInt8(states[state], offset + 2); // id
    offset += 3;

    // appVersion
    packet.writeUInt8(0x0d, offset); // id
    packet.writeUInt8(0x02, offset + 1); // length
    packet.writeUInt16BE(appVersion, offset + 2);
    offset += 4;

    // log
    packet.writeUInt8(0x8c, offset); // type
    packet.writeUInt8(0x00, offset + 1); // reserved
    packet.writeUInt16BE(logBuffer.length, offset + 2);
    offset += 4;
    for (var i=0; i < logBuffer.length; i++) {
      packet.writeUInt8(logBuffer[i], offset);
      offset++;
    }

    return new Promise((resolve, reject) => {
      fetch(`${this.url}/api/v1/heartbeat`, {
        method: "POST",
        body: packet
      }).then(response => {
        if (200 <= response.status && response.status <= 299)
        resolve(response);
        else
        reject(response);
      });
    });
  }

  run() {
    this.resetAPIs();

    this.sendHeartbeat("booting", 0, "");
    setInterval(() => {
      console.log("heartbeating...");

      this.sendHeartbeat("running", this.appVersion, this.apis.logging.getLog()).then(r => {
        r.buffer().then(buffer => {
          let r = this.parseMspPacket(buffer);
          let { appUpdateRequest, appImageURL, stores } = r;

          if (appUpdateRequest) {
            console.log(`updating ${this.appVersion} -> ${r.appVersion}`);
            this.appVersion = r.appVersion;
            fetch(appImageURL).then(r => {
              r.text().then(script => {
                this.runScript(script);
              });
            });
          }

          this.apis.store.updateStores(stores);
        });
      });
    }, 3000);
  }

  runScript(script) {
    const vm = new NodeVM({
      sandbox: this.globals,
      require: false,
      console: 'inherit',
      timeout: 5000
    });

    vm.run(script);
  }
}