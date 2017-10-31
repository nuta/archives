import 'whatwg-fetch'

export default new class {
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
          if (response.status === 401 && app.$router.currentRoute.name !== 'login') {
            this.forceLogin((app.$router.currentRoute.name === 'home') ? null : 'Login first.')
          }
          return response
        })
        .then(response => {
          return response.json()
        })
        .then(resolve)
        .catch(error => {
          alert(`something went wrong :(\n\n${error.stack}`)
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
    return this.invoke('POST', '/auth/sign_in', {
      username: username,
      password: password
    }, false).then(r => {
      this.user = {
        username: username,
        email: r.json['data']['email']
      }

      this.credentials = {
        uid: r.headers.get('uid'),
        'access-token': r.headers.get('access-token'),
        'access-token-secret': r.headers.get('access-token-secret')
      }

      localStorage.setItem('credentials', JSON.stringify(this.credentials))
      localStorage.setItem('user', JSON.stringify(this.user))
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
      redirect_url: '/home'
    }, false)
  }

  getApps() {
    return this.invoke('GET', `/apps`)
  }

  getApp(appName) {
    return this.invoke('GET', `/apps/${appName}`)
  }

  getAppLog(appName, since = 0) {
    return this.invoke('GET', `/apps/${appName}/log?since=${since}`)
  }

  updateApp(appName, attrs) {
    return this.invoke('PUT', `/apps/${appName}`, attrs)
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
}()
