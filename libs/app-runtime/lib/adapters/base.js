const { msgpack } = require('hyperutils')

const SMMS_VERSION_MSG = 1;
const SMMS_APP_UPDATE_REQUEST_MSG = 0x11
const SMMS_DEVICE_ID_MSG = 0x0a
const SMMS_DEVICE_INFO_MSG = 0x0b
const SMMS_LOG_MSG = 0x0c
const SMMS_APP_VERSION_MSG = 0x0d
const SMMS_STORE_MSG = 0x40
const SMMS_STORE_MSG_END = 0x7f

class AdapterBase {
  constructor() {
    this.onReceiveCallback = () => { }
  }

  onReceive(callback) {
    this.onReceiveCallback = callback
  }

  serialize(messages) {
    const { state, appVersion, log } = messages;
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

    return msgpack.pack(payload)
  }
  
  deserialize(payload) {
    let stores = {};

    let data = msgpack.unpack(payload);
    let appUpdateRequest = data[SMMS_APP_UPDATE_REQUEST_MSG];
    
    for (let k in msgpack.unpack(payload)) {
      if (SMMS_STORE_MSG <= k && k < SMMS_STORE_MSG_END)
        stores[data[k][0]] = data[k][1];
    }
    
    return { appUpdateRequest, stores };
  }
}

module.exports = AdapterBase