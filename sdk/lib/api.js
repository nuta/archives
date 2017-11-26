const fs = require('fs')
const util = require('util')
const FormData = require('form-data')
const fetch = require('node-fetch')
const { loadCredentials, saveCredentials } = require('./config')

class API {
  invoke(method, path, body) {
    const headers = {}

    return new Promise((resolve, reject) => {
      const credentials = loadCredentials()
      if (!credentials) {
        reject(new Error('login first'))
      }

      if (!(body instanceof FormData)) {
        body = JSON.stringify(body)
        Object.assign(headers, {
          'Content-Type': 'application/json'
        })
      }

      let status
      fetch(`${credentials.url}/api/v1${path}`, {
        method: method,
        headers: Object.assign(headers, credentials),
        body
      }).then(response => {
        status = response.status
        return (status === 204) ? Promise.resolve({}) : response.json()
      }).then(json => {
        if (!(status >= 200 && status <= 300)) {
          const msg = util.inspect(json)
          reject(Error(`Error: server returned ${status}: \`${msg}'`))
        }

        resolve(json)
      })
    })
  }

  get serverURL() {
    return loadCredentials().url
  }

  logout() {
    fs.unlinkSync(this.credentialsPath)
  }

  login(url, username, password) {
    let status, headers
    return fetch(`${url}/api/v1/auth/sign_in`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ username, password })
    }).then((response) => {
      status = response.status
      headers = response.headers
      return response.json()
    }).then(json => {
      if (status !== 200) {
        throw new Error(`Error: failed to login: \`${json.errors}'`)
      }

      saveCredentials({
        url,
        username,
        email: json['data']['email'],
        uid: headers.get('uid'),
        'access-token': headers.get('access-token'),
        'access-token-secret': headers.get('access-token-secret')
      })
    })
  }

  getApps() {
    return this.invoke('GET', `/apps`)
  }

  createApp(appName, api) {
    return this.invoke('POST', `/apps`, {
      app: { name: appName, api: api }
    })
  }

  getFiles(appName) {
    return this.invoke('GET', `/apps/${appName}/files`)
  }

  saveFile(appName, path, body) {
    return this.invoke('PUT', `/apps/${appName}/files/${path}`, { body })
  }

  getApp(appName) {
    return this.invoke('GET', `/apps/${appName}`)
  }

  editApp(appName, attrs) {
    return this.invoke('PUT', `/apps/${appName}`, attrs)
  }

  getDeployments(appName) {
    return this.invoke('GET', `/apps/${appName}/deployments`)
  }

  getDeployment(appName, version) {
    return this.invoke('GET', `/apps/${appName}/deployments/${version}`)
  }

  deploy(appName, image, debug, comment, tag) {
    const form = new FormData()
    form.append('deployment[deployed_at]', 'client')
    form.append('deployment[image]', image)
    return this.invoke('POST', `/apps/${appName}/deployments`, form)
  }

  deleteApp(appName) {
    return this.invoke('DELETE', `/apps/${appName}`)
  }

  getDevices() {
    return this.invoke('GET', `/devices`)
  }

  getDevice(deviceName) {
    return this.invoke('GET', `/devices/${deviceName}`)
  }

  getDeviceLog(deviceName) {
    return this.invoke('GET', `/devices/${deviceName}/log`)
  }

  getDeviceStores(deviceName) {
    return this.invoke('GET', `/devices/${deviceName}/stores`)
  }

  setDeviceStore(deviceName, key, value) {
    return this.invoke('PUT', `/devices/${deviceName}/stores/${key}`, { value })
  }

  registerDevice(name, deviceType, tag) {
    return this.invoke('POST', '/devices', { device: { name, device_type: deviceType, tag } })
  }

  deleteDevice(deviceName) {
    return this.invoke('DELETE', `/devices/${deviceName}`)
  }

  getAppStores(appName) {
    return this.invoke('GET', `/apps/${appName}/stores`)
  }

  setAppStore(appName, key, value) {
    return this.invoke('PUT', `/apps/${appName}/stores/${key}`, { value })
  }

  downloadPlugin(name) {
    let repo
    if (name.includes('/')) {
      // A third-party plugin on GitHub.
      repo = name
      name = 'nodejs-' + name.split('/').pop()
    } else {
      repo = '_/_'
    }

    return new Promise((resolve, reject) => {
      fetch(`${this.serverURL}/api/v1/plugins/${repo}/${name}`, {
        headers: loadCredentials()
      }).then(response => {
        response.buffer().then(resolve, reject)
      })
    })
  }

  getAppLog(appName, since) {
    const unixtime = since ? Math.floor(since.getTime() / 1000) : 0
    return this.invoke('GET', `/apps/${appName}/log?since=${unixtime}`)
  }

  async streamAppLog(appName, callback) {
    callback((await this.getAppLog(appName)).lines)

    let lastFetchedAt = null
    setInterval(async () => {
      callback((await this.getAppLog(appName, lastFetchedAt)).lines)
      lastFetchedAt = new Date()
    }, 5000)
  }

  getOSReleases() {
    return this.invoke('GET', '/os/releases')
  }

  invokeCommand(deviceName, command, arg) {
    return this.invoke('POST', `/devices/${deviceName}/commands`, { command, arg })
  }

  getCommandResults(deviceName) {
    return this.invoke('GET', `/devices/${deviceName}/commands`)
  }
}

module.exports = new API()
