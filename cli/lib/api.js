const fs = require('fs')
const util = require('util')
const FormData = require('form-data')
const fetch = require('node-fetch')
const config = require('./config')

class API {
  doInvoke(method, path, headers, body) {
    return new Promise((resolve, reject) => {
      if (!config.credentials) {
        reject(new Error('login first'))
      }

      let respStatus, respHeaders
      fetch(`${config.server.url}/api/v1${path}`, {
        method: method,
        headers:  Object.assign(headers, config.credentials),
        body
      }).then(response => {
        respStatus = response.status
        respHeaders = response.headers
        return response
      }).then(response => {
        return response.json()
      }).then(json => {
        const result = { respStatus, respHeaders, json }
        if (respStatus === 200 || respStatus === 201) {
          resolve(result)
        } else {
          console.error(`server returned ${respStatus}\n`)
          console.error(util.inspect(json, false, 8))
        }
      })
    })
  }

  invoke(method, path, params) {
    const body = JSON.stringify(params)
    const headers = {
      'Content-Type': 'application/json'
    }

    return this.doInvoke(method, path, headers, body)
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

  signup(username, email, password) {
    return this.invoke('POST', '/auth', {
      name: username,
      email: email,
      password: password
    }, false)
  }

  deleteUser(username) {
    return this.invoke('DELETE', '/auth', {
      name: username
    }, false)
  }

  resetPassword(email) {
    return this.invoke('POST', '/auth/password', {
      email: email,
      redirectUrl: '/home'
    }, false)
  }

  getApps() {
    return this.invoke('GET', `/apps`)
  }

  getApp(appName) {
    return this.invoke('GET', `/apps/${appName}`)
  }

  updateApp(appName, attrs) {
    return this.invoke('PUT', `/apps/${appName}`, attrs)
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

  updateDeviceStore(deviceName, key, value) {
    return this.invoke('PUT', `/devices/${deviceName}/stores/${key}`, { value })
  }

  registerDevice(name, deviceType, tag) {
    return this.invoke('POST', '/devices', { device: { name, device_type: deviceType, tag } })
  }

  deleteDevice(deviceName) {
    return this.invoke('DELETE', `/devices/${deviceName}`)
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
    return this.doInvoke('POST', `/apps/${appName}/deployments`, {}, form)
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

  getAppStores(appName) {
    return this.invoke('GET', `/apps/${appName}/stores`)
  }

  updateAppStore(appName, key, value) {
    return this.invoke('PUT', `/apps/${appName}/stores/${key}`, { value })
  }
}

module.exports = new API()
