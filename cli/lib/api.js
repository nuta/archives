const fs = require('fs')
const FormData = require('form-data')
const fetch = require('node-fetch')
const config = require('./config')

class API {
  invoke(method, path, headers, body) {
    return new Promise((resolve, reject) => {
      if (!config.credentials) {
        reject(new Error('login first'))
      }

      if (!(body instanceof FormData)) {
        body = JSON.stringify(body)
        Object.assign(headers, {
          'Content-Type': 'application/json'
        })
      }

      fetch(`${config.server.url}/api/v1${path}`, {
        method: method,
        headers: Object.assign(headers, config.credentials),
        body
      }).then(response => {
        response.json().then(resolve).catch(reject)
      })
    })
  }

  get serverURL() {
    return config.server.url
  }

  logout() {
    fs.unlinkSync(this.credentialsPath)
  }

  login(url, username, password) {
    config.server = { url }
    return this.invoke('POST', '/auth/sign_in', {
      username: username,
      password: password
    }, false).then(r => {
      config.credentials = {
        username: username,
        email: r.json['data']['email'],
        uid: r.headers.get('uid'),
        'access-token': r.headers.get('access-token'),
        'access-token-secret': r.headers.get('access-token-secret')
      }
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
    form.append('deployment[image]', image)
    return this.invoke('POST', `/apps/${appName}/deployments`, {}, form)
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
}

module.exports = new API()
