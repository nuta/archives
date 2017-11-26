const path = require('path')
const fetch = require('node-fetch')
const fs = require('fs')

const CONFIG_DIR = process.env.CONFIG_DIR || `${process.env.HOME}/.makestack`
const CREDENTIALS_JSON = `${CONFIG_DIR}/credentials.json`

function loadCredentials(name) {
  try {
    return JSON.parse(fs.readFileSync(CREDENTIALS_JSON))
  } catch (e) {
    return null
  }
}

function saveCredentials(data) {
  fs.mkdirSync(path.dirname(CREDENTIALS_JSON))
  fs.writeFileSync(CREDENTIALS_JSON, JSON.stringify(data))
}

module.exports = new class {
  constructor() {
    this.credentials = loadCredentials()
  }

  invoke(method, server, path, params, requiresCredentials = true) {
    let reqHeaders = Object.assign({
      'Content-Type': 'application/json'
    }, this.credentials)

    return new Promise((resolve, reject) => {
      let status
      let headers
      fetch(`${this.credentials.url}/api/v1${path}`, {
        method: method,
        headers: reqHeaders,
        body: JSON.stringify(params)
      }).then(response => {
        status = response.status
        headers = response.headers
        return response
      }).then(response => {
        return response.json()
      }).then(json => {
        if (status >= 200 && status <= 299) {
          resolve({ status, headers, json })
        } else {
          reject(new Error(`server returned ${status}`))
        }
      })
    })
  }

  login(url, username, password) {
    return this.invoke('POST', url, '/auth/sign_in', {
      username: username,
      password: password
    }, false).then(r => {
      this.credentials = {
        username: username,
        email: r.json['data']['email'],
        uid: r.headers.get('uid'),
        'access-token': r.headers.get('access-token'),
        'access-token-secret': r.headers.get('access-token-secret')
      }

      saveCredentials(this.credentials)
    })
  }

  isLoggedIn() {
    return this.credentials !== null
  }

  updateDeviceStore(deviceName, key, value) {
    return this.invoke('PUT', this.credentials.url, `/devices/${deviceName}/stores/${key}`,
      { value })
  }

  registerDevice(name, deviceType, tag) {
    return this.invoke('POST', this.credentials.url, '/devices',
      { device: { name, device_type: deviceType, tag } })
  }
}()
