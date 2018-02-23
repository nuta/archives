import 'whatwg-fetch'
import {
  getServerUrl,
  loadCredentials,
  saveCredentials,
  removeCredentials
} from 'platform'

class API {
  constructor() {
    this.credentials = loadCredentials()
    if (this.credentials && Object.keys(this.credentials).length > 0) {
      this.server = getServerUrl()
    } else {
      this.credentials = null
    }
  }

  invoke(method, path, body) {
    let headers = Object.assign({}, this.credentials)

    if (!this.credentials) {
      this.forceLogin()
    }

    if (typeof body !== 'string' && !(body instanceof FormData)) {
      body = JSON.stringify(body)
      Object.assign(headers, { 'Content-Type': 'application/json' })
    }

    return new Promise((resolve, reject) => {
      let status
      fetch(`${this.server}/api/v1${path}`, { method, headers, body })
        .then(response => {
          if (response.status === 401 && path !== '/login') {
            this.forceLogin()
          }
          return response
        })
        .then(response => {
          if (response.status !== 422 && !(response.status >= 200 && response.status < 300)) {
            throw new Error(`server returned ${response.status}`)
          }
          return response
        })
        .then(response => {
          status = response.status
          return (response.status === 204) ? Promise.resolve({}) : response.json()
        })
        .then(json => {
          if (status === 422) {
            reject(new Error(json.errors[0]))
          } else {
            resolve(json)
          }
        })
        .catch(e => {
          reject(e)
        })
    })
  }

  get serverUrl() {
    return (this.server === '') ? location.origin : this.server
  }

  get username() {
    return this.credentials ? this.credentials.username : null
  }

  get email() {
    return this.credentials ? this.credentials.email : null
  }

  loggedIn() {
    return this.credentials !== null
  }

  logout() {
    removeCredentials()
  }

  forceLogin(errmsg) {
    this.credentials = null
    this.logout()
  }

  login(server, username, password) {
    let status, headers
    return fetch(`${server}/api/v1/auth/sign_in`, {
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

      this.server = server
      this.credentials = {
        url: this.server,
        username: username,
        email: json['data']['email'],
        uid: headers.get('uid'),
        'access-token': headers.get('access-token'),
        'access-token-secret': headers.get('access-token-secret')
      }

      saveCredentials(this.credentials)
    })
  }

  createUser({ server, username, email, password, passwordConfirmation, recaptcha, agreeTos }) {
    let status
    return fetch(`${server}/api/v1/auth`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        username,
        email,
        password,
        password_confirmation: passwordConfirmation,
        recaptcha,
        agree_tos: agreeTos
      })
    }).then((response) => {
      status = response.status
      return response.json()
    }).then(json => {
      if (status !== 200) {
        throw new Error(`Error: failed to create an account: ${json.errors.full_messages[0]}`)
      }

      this.server = server
    })
  }

  resetPassword(server, email) {
    let status
    return fetch(`${server}/api/v1/auth/password`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ email })
    }).then((response) => {
      status = response.status
      return response.json()
    }).then(json => {
      if (status !== 200) {
        throw new Error(`Error: failed to reset password: \`${json.errors}'`)
      }
    })
  }

  deleteUser(ensureThatCallerWantsToDeleteUser) {
    if (ensureThatCallerWantsToDeleteUser !== 'Hey, please delete the user account!') {
      throw new Error('BUG: api.deleteUser() has been accidentally called.')
    }

    return this.invoke('DELETE', '/auth')
  }

  updateUser(attrs) {
    // It is necessary to update credentials in local storage
    // when the email address has been changed.
    return this.invoke('PUT', '/auth', attrs)
  }

  getApps() {
    return this.invoke('GET', `/apps`)
  }

  getApp(appName) {
    return this.invoke('GET', `/apps/${appName}`)
  }

  getAppLog(appName, since) {
    const unixtime = since ? Math.floor(since.getTime() / 1000) : 0
    return this.invoke('GET', `/apps/${appName}/log?since=${unixtime}`)
  }

  updateApp(appName, attrs) {
    return this.invoke('PUT', `/apps/${appName}`, { app: attrs })
  }

  deleteApp(appName) {
    return this.invoke('DELETE', `/apps/${appName}`)
  }

  getIntegrations(appName) {
    return this.invoke('GET', `/apps/${appName}/integrations`)
  }

  createIntegration(appName, service, config, comment) {
    return this.invoke('POST', `/apps/${appName}/integrations`,
      { service, config: JSON.stringify(config), comment })
  }

  updateIntegration(appName, name, service, config) {
    return this.invoke('PUT', `/apps/${appName}/integrations/${name}`,
      { service, config: JSON.stringify(config), comment: '' })
  }

  deleteIntegration(appName, name) {
    return this.invoke('DELETE', `/apps/${appName}/integrations/${name}`)
  }

  getAppDevices(appName) {
    return new Promise((resolve, reject) => {
      this.getDevices().then(devices => {
        resolve(devices.filter(device => device.app === appName))
      })
    })
  }

  getDevices() {
    return this.invoke('GET', `/devices`)
  }

  getDevice(deviceName) {
    return this.invoke('GET', `/devices/${deviceName}`)
  }

  updateDevice(deviceName, attrs) {
    return this.invoke('PATCH', `/devices/${deviceName}`, { device: attrs })
  }

  getDeviceLog(deviceName) {
    return this.invoke('GET', `/devices/${deviceName}/log`)
  }

  deleteDeviceConfig(deviceName, key) {
    return this.invoke('DELETE', `/devices/${deviceName}/configs/${key}`)
  }

  getDeviceConfigs(deviceName) {
    return this.invoke('GET', `/devices/${deviceName}/configs`)
  }

  updateDeviceConfig(deviceName, key, type, value) {
    return this.invoke('PUT', `/devices/${deviceName}/configs/${key}`,
      { data_type: type, value })
  }

  deleteDevice(deviceName) {
    return this.invoke('DELETE', `/devices/${deviceName}`)
  }

  invokeCommand(deviceName, command, arg = []) {
    return this.invoke('POST', `/devices/${deviceName}/commands`, { command, arg })
  }

  rebootDevice(deviceName) {
    return this.invokeCommand(deviceName, '__reboot__')
  }

  getDeployments(appName) {
    return this.invoke('GET', `/apps/${appName}/deployments`)
  }

  getDeployment(appName, version) {
    return this.invoke('GET', `/apps/${appName}/deployments/${version}`)
  }

  deploy(appName, image, debug, comment, tag) {
    let form = new FormData()
    form.set('deployment[deployed_from]', 'web')
    form.set('deployment[image]', image, 'app.zip')

    if (debug) {
      form.set('deployment[debug]', debug)
    }

    if (comment) {
      form.set('deployment[comment]', comment)
    }

    if (tag) {
      form.set('deployment[tag]', tag)
    }

    return this.invoke('POST', `/apps/${appName}/deployments`, form)
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

  getAppConfigs(appName) {
    return this.invoke('GET', `/apps/${appName}/configs`)
  }

  createAppConfig(appName, key, type, value) {
    return this.invoke('POST', `/apps/${appName}/configs`, {
      config: { key, data_type: type, value }
    })
  }

  updateAppConfig(appName, key, type, value) {
    return this.invoke('PUT', `/apps/${appName}/configs/${key}`,
      { config: { data_type: type, value } })
  }

  deleteAppConfig(appName, key) {
    return this.invoke('DELETE', `/apps/${appName}/configs/${key}`)
  }

  getOSReleases() {
    return this.invoke('GET', '/os/releases')
  }
}

export default new API()
