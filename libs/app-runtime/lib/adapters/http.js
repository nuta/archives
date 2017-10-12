const fetch = require('node-fetch')
const AdapterBase = require('./base')

class HTTPAdapter extends AdapterBase {
  constructor(serverURL, deviceId) {
    super()
    
    this.serverURL = serverURL
    this.deviceId = deviceId
  }
  
  send(messages) {
    fetch(`${this.serverURL}/api/v1/heartbeat`, {
      method: "POST",
      body: this.serialize(messages)
    }).then(response => {
      if (response.status == 200) {
        response.buffer().then(buffer => {
          this.onReceiveCallback(this.deserialize(buffer))
        })
      } else {
        reject(response)
      }
    })
  }

  getAppImage(version) {
    return new Promise((resolve, reject) => {
      let appImageserverURL = `${this.serverURL}/api/v1/app_image?device_id=${this.deviceId}&version=${version}`
      
          fetch(appImageserverURL).then(r => {
            r.text().then(resolve).catch(reject)
          })
    })
  }
}

module.exports = HTTPAdapter