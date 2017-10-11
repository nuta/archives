const os = require('os');
const { fetch, msgpack, NodeVM } = require('hyperutils');
const LoggingAPI = require('./logging');
const TimerAPI = require('./timer');
const StoreAPI = require('./store');
const EventAPI = require('./event');

let GPIOAPI, I2CAPI;
switch(os.type()) {
  case 'Linux':
    GPIOAPI = require('./linux/gpio');
    I2CAPI = require('./linux/i2c');
    break;
  default:
    GPIOAPI = require('./mock/gpio');
    I2CAPI = require('./mock/i2c');
    break;
}

const SMMS_VERSION_MSG = 1;
const SMMS_APP_UPDATE_REQUEST_MSG = 0x11;
const SMMS_DEVICE_ID_MSG = 0x0a;
const SMMS_DEVICE_INFO_MSG = 0x0b;
const SMMS_LOG_MSG = 0x0c;
const SMMS_APP_VERSION_MSG = 0x0d;
const SMMS_STORE_MSG = 0x40;
const SMMS_STORE_MSG_END = 0x7f;

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
  
  parseSMMSPayload(payload) {
    let stores = {};

    let data = msgpack.unpack(payload);
    let appUpdateRequest = data[SMMS_APP_UPDATE_REQUEST_MSG];
    
    for (let k in msgpack.unpack(payload)) {
      if (SMMS_STORE_MSG <= k && k < SMMS_STORE_MSG_END)
        stores[data[k][0]] = data[k][1];
    }
    
    return { appUpdateRequest, stores };
  }

  sendHeartbeat(state, appVersion, log) {
    const states = { new: 1, booting: 2, ready: 3, running: 4, down: 5, reboot: 6, relaunch: 7 }
    if (!states[state]) {
      throw `Invalid device state: \`${state}'`;
    }

    let payload = {}
    payload[SMMS_VERSION_MSG] = 1
    payload[SMMS_DEVICE_INFO_MSG] = states[state]
    payload[SMMS_DEVICE_ID_MSG] = this.deviceId
    payload[SMMS_APP_VERSION_MSG] = appVersion
    payload[SMMS_LOG_MSG] = log

    return new Promise((resolve, reject) => {
      fetch(`${this.url}/api/v1/heartbeat`, {
        method: "POST",
        body: msgpack.pack(payload)
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
          let r = this.parseSMMSPayload(buffer);
          let { appUpdateRequest, stores } = r;

          if (appUpdateRequest) {
            console.log(`updating ${this.appVersion} -> ${appUpdateRequest}`);
            this.appVersion = appUpdateRequest;
            let appImageURL = `${this.url}/api/v1/app_image?device_id=${this.deviceId}&version=${appUpdateRequest}`;
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