const fetch = require('node-fetch')
const AdapterBase = require('./base')

class HTTPAdapter extends AdapterBase {
  constructor(deviceId, serverURL) {
    super()

    this.deviceId = deviceId
    this.serverURL = serverURL
  }

  connect() {
    // Nothing to do.
  }

  send(payload) {
    fetch(`${this.serverURL}/api/v1/smms`, {
      method: 'POST',
      body: payload
    }).then(response => {
      if (response.status === 200) {
        response.buffer().then(this.onReceiveCallback)
      } else {
        console.error('server returned error:', response)
      }
    })
  }

  getAppImage(version) {
    return new Promise((resolve, reject) => {
      let url = `${this.serverURL}/api/v1/images/app/${this.deviceId}/${version}`

      fetch(url).then(response => {
        response.buffer().then(resolve)
      })
    })
  }

  getOSImage(deviceType, version) {
    return new Promise((resolve, reject) => {
      let url = `${this.serverURL}/api/v1/images/os/${this.deviceId}/${version}/linux/${deviceType}`

      fetch(url).then(response => {
        response.buffer().then(resolve)
      })
    })
  }
}

module.exports = HTTPAdapter
