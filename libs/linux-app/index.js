const fetch = require('node-fetch');
const { NodeVM } = require('vm2');
const LoggingAPI = require('./logging');
const TimerAPI = require('./timer');
const StoreAPI = require('./store');
const EventAPI = require('./event');
const GPIOAPI = require('./gpio');

module.exports = class {
  constructor() {
    this.context = {};
    this.apis = {};
    this.globals = {};

    let loggingApi = this.registerAPI("logging", LoggingAPI);
    this.registerAPI("timer", TimerAPI);
    this.registerAPI("event", EventAPI, loggingApi);
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

  run(script) {
    this.resetAPIs();
  
    const vm = new NodeVM({
      sandbox: this.globals,
      require: false,
      console: 'inherit',
      timeout: 5000
    });
  
    vm.run(script);
  }
}