const fetch = require('node-fetch')
const AdapterBase = require('./base')

class HTTPAdapter extends AdapterBase {
  constructor(deviceId, serverURL) {
    super()

    this.serverURL = serverURL
    this.deviceId = deviceId
  }

  send(messages) {
    fetch(`${this.serverURL}/api/v1/smms`, {
      method: 'POST',
      body: this.serialize(messages)
    }).then(response => {
      if (response.status === 200) {
        response.buffer().then(buffer => {
          this.onReceiveCallback(this.deserialize(buffer))
        })
      } else {
        console.error('server returned error:', response)
      }
    })
  }

  getAppImage(version) {
    return new Promise((resolve, reject) => {
      let appImageserverURL = `${this.serverURL}/api/v1/images/app/${this.deviceId}/${version}`

      fetch(appImageserverURL).then(r => {
        r.buffer().then(resolve).catch(reject)
      })
    })
  }

  getOSImage(deviceType, version) {
    return new Promise((resolve, reject) => {
      let url = `${this.serverURL}/api/v1/images/os/linux/${version}/${deviceType}`

      fetch(url).then(r => {
        r.buffer().then(resolve).catch(reject)
      })
    })
  }
}

module.exports = HTTPAdapter
