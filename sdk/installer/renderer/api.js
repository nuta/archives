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
  if (!fs.existsSync(path.dirname(CREDENTIALS_JSON))) {
    fs.mkdirSync(path.dirname(CREDENTIALS_JSON))
  }

  fs.writeFileSync(CREDENTIALS_JSON, JSON.stringify(data))
}

module.exports = new class {
  constructor() {
    this.reloadCredentials()
  }

  reloadCredentials() {
    this.credentials = loadCredentials()
    return this.credentials
  }

  invoke(method, path, params) {
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
          alert(`server returned ${status}\n${json}`)
        }
      })
    })
  }

  login(url, username, password) {
    let headers, status
    return fetch(`${url}/api/v1/auth/sign_in`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ username, password })
    })
      .then(response => {
        status = response.status
        headers = response.headers
        return response.json()
      })
      .then(json => {
        if (status !== 200) {
          if (json.errors[0] === 'Invalid login credentials. Please try again.') {
            alert('Incorrect username or password.')
            return
          }

          throw new Error(`failed to login (${JSON.stringify(json)})`)
        }

        saveCredentials({
          url,
          username,
          email: json['data']['email'],
          uid: headers.get('uid'),
          'access-token': headers.get('access-token'),
          'access-token-secret': headers.get('access-token-secret')
        })

        loadCredentials()
      })
  }

  isLoggedIn() {
    return this.reloadCredentials() !== null
  }

  updateDeviceStore(deviceName, key, value) {
    return this.invoke('PUT', `/devices/${deviceName}/stores/${key}`, { value })
  }

  registerDevice(name, deviceType, tag) {
    return this.invoke('POST', '/devices', {
      device: { name, device_type: deviceType, tag }
    })
  }
}()
