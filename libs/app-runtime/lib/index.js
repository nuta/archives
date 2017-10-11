const os = require('os');
const { NodeVM } = require('hyperutils');
const LoggingAPI = require('./logging');
const TimerAPI = require('./timer');
const StoreAPI = require('./store');
const EventAPI = require('./event');
const HTTPAdapter = require('./adapters/http')

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

module.exports = class {
  constructor(url, deviceId, deviceSecret) {
    this.deviceId = deviceId;
    this.deviceSecret = deviceSecret;
    this.appVersion = 0;
    this.context = {};
    this.apis = {};
    this.globals = {};
    this.adapter = new HTTPAdapter(url, deviceId)

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

  run() {
    this.resetAPIs();
    this.adapter.send({
      state: "booting",
      appVersion: 0,
      log: ""
    })

    this.adapter.onReceive(({ appUpdateRequest, stores }) => {
      if (appUpdateRequest) {
        console.log(`updating ${this.appVersion} -> ${appUpdateRequest}`)
        this.appVersion = appUpdateRequest
        this.adapter.getAppImage(appUpdateRequest).then((script) => {
          this.runScript(script)
        })
      }

      this.apis.store.updateStores(stores);
    })
  
    setInterval(() => {
      console.log("heartbeating...");
      let log = this.apis.logging.getLog()
      this.adapter.send({
        state: "running",
        appVersion: this.appVersion,
        log: log
      })
    }, 3000);
  }

  runScript(script) {
    const vm = new NodeVM({
      sandbox: this.globals,
      require: false,
      console: 'inherit',
      timeout: 5000
    });

    try {
      vm.run(script);
    } catch (e) {
      let msg = e.stack.replace(/^/gm, '!');
      this.apis.logging.print(msg);
    }
  }
}