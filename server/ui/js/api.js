import 'whatwg-fetch'

class API {
  constructor() {
    let dummyUser = { name: '', email: '' }
    this.user = JSON.parse(localStorage.getItem('user')) || dummyUser
    this.credentials = JSON.parse(localStorage.getItem('credentials'))
  }

  invoke(method, path, body, requiresCredentials = true) {
    let headers = Object.assign({}, this.credentials)

    if (requiresCredentials && (!this.user || !this.credentials)) {
      this.forceLogin()
    }

    if (typeof body !== 'string' && !(body instanceof FormData)) {
      body = JSON.stringify(body)
      Object.assign(headers, { 'Content-Type': 'application/json' })
    }

    return new Promise((resolve, reject) => {
      fetch(`/api/v1${path}`, { method, headers, body })
        .then(response => {
          const currentRoute = app.$router.currentRoute.name
          if (response.status === 401 && currentRoute !== 'login') {
            this.forceLogin((currentRoute === 'home') ? null : 'Login first.')
          }
          return response
        })
        .then(response => {
          if (!(response.status >= 200 && response.status < 300)) {
            throw new Error(`server returned ${response.status}`)
          }
          return response
        })
        .then(response => {
          return (response.status === 204) ? Promise.resolve({}) : response.json()
        })
        .then(resolve)
        .catch(e => {
          UIkit.notification(e.toString(), { status: 'danger' })
          console.error(e)
        })
    })
  }

  logout() {
    localStorage.removeItem('user')
    localStorage.removeItem('credentials')
  }

  forceLogin(errmsg) {
    this.logout()
    app.$router.push({name: 'login'})
  }

  login(username, password) {
    let status, headers
    return fetch(`/api/v1/auth/sign_in`, {
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

      this.user = {
        username: username,
        email: json['data']['email']
      }

      this.credentials = {
        uid: headers.get('uid'),
        'access-token': headers.get('access-token'),
        'access-token-secret': headers.get('access-token-secret')
      }

      localStorage.setItem('credentials', JSON.stringify(this.credentials))
      localStorage.setItem('user', JSON.stringify(this.user))
    })
  }

  createUser({ username, email, password, passwordConfirmation, recaptcha, agreeTos }) {
    return this.invoke('POST', '/auth', {
      username,
      email,
      password,
      password_confirmation: passwordConfirmation,
      recaptcha,
      agree_tos: agreeTos
    }, false)
  }

  getUser(username) {
    return this.invoke('GET', '/auth', {
      name: username
    }, false)
  }

  deleteUser() {
    return this.invoke('DELETE', '/auth')
  }

  updateUser(attrs) {
    // It is necessary to update credentials in local storage
    // when the email address has been changed.
    return this.invoke('PUT', '/auth', attrs)
  }

  resetPassword(email) {
    return this.invoke('POST', '/auth/password', { email }, false)
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
      { service, config, comment })
  }

  updateIntegration(appName, name, service, config, comment) {
    return this.invoke('PUT', `/apps/${appName}/integrations/${name}`,
      { service, config, comment })
  }

  deleteIntegration(appName, name) {
    return this.invoke('DELETE', `/apps/${appName}/integrations/${name}`)
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

  createDeviceStore(deviceName, key, type, value) {
    return this.invoke('POST', `/devices/${deviceName}/stores`, {
      store: { key, data_type: type, value }
    })
  }

  deleteDeviceStore(deviceName, key) {
    return this.invoke('DELETE', `/devices/${deviceName}/stores/${key}`)
  }

  getDeviceStores(deviceName) {
    return this.invoke('GET', `/devices/${deviceName}/stores`)
  }

  updateDeviceStore(deviceName, key, value) {
    return this.invoke('PUT', `/devices/${deviceName}/stores/${key}`,
      { value })
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

  getAppStores(appName) {
    return this.invoke('GET', `/apps/${appName}/stores`)
  }

  createAppStore(appName, key, type, value) {
    return this.invoke('POST', `/apps/${appName}/stores`, {
      store: { key, data_type: type, value }
    })
  }

  updateAppStore(appName, key, type, value) {
    return this.invoke('PUT', `/apps/${appName}/stores/${key}`,
      { store: { data_type: type, value } })
  }

  deleteAppStore(appName, key) {
    return this.invoke('DELETE', `/apps/${appName}/stores/${key}`)
  }

  getOSReleases() {
    return this.invoke('GET', '/os/releases')
  }
}

export default new API()
