import "whatwg-fetch";

export default new class {
  constructor() {
    let dummyUser = { name: "", email: "" };
    this.user    = JSON.parse(localStorage.getItem("user")) || dummyUser;
    this.credentials = JSON.parse(localStorage.getItem("credentials"));
  }

  invoke(method, path, params, requiresCredentials=true) {
    let reqHeaders = Object.assign({
      "Content-Type": "application/json"
    }, this.credentials);

    if (requiresCredentials && (!this.user || !this.credentials)) {
      this.forceLogin();
    }

    return new Promise((resolve, reject) => {
      let status;
      let headers;
      fetch(`/api/v1${path}`, {
        method: method,
        headers: reqHeaders,
        body: JSON.stringify(params)
      }).then((response) => {
        status = response.status;
        headers = response.headers;
        return response;
      }).then((response) => {
        if (response.status === 401 && app.$router.currentRoute.name !== "login") {
          this.forceLogin((app.$router.currentRoute.name === "home") ? null : "Login first.");
        }
        return response;
      }).then((response) => {
        return response.json();
      }).catch(error => {
        reject({ status, headers, error });
      }).then((json) => {
        if (200 <= status && status <= 299)
          resolve({status, headers, json });
        else
          reject({status, headers, json });
      });
    });
  }

  logout() {
    localStorage.removeItem("user");
    localStorage.removeItem("credentials");
  }

  login(username, password) {
    return this.invoke("POST", "/auth/sign_in", {
      username: username,
      password: password
    }, false).then(r => {
      this.user = {
        username: username,
        email: r.json["data"]["email"]
      };

      this.credentials = {
        uid:           r.headers.get("uid"),
        "access-token":    r.headers.get("access-token"),
        "access-token-secret": r.headers.get("access-token-secret")
      };
  
      localStorage.setItem("credentials", JSON.stringify(this.credentials));   
      localStorage.setItem("user", JSON.stringify(this.user));
    });
  }

  forceLogin(errmsg) {
    this.logout();
    app.$router.push({name: "login"});
  }

  signup(username, email, password) {
    return this.invoke("POST", "/auth", {
      name: username,
      email: email,
      password: password
    }, false);
  }

  deleteUser(username) {
    return this.invoke("DELETE", "/auth", {
      name: username
    }, false);
  }

  resetPassword(email) {
    return this.invoke("POST", "/auth/password", {
      email: email,
      redirect_url: "/home"
    }, false);
  }

  getApps() {
    return this.invoke("GET", `/apps`);
  }

  getApp(appName) {
    return this.invoke("GET", `/apps/${appName}`);
  }

  getAppLog(appName, since = 0) {
    return this.invoke("GET", `/apps/${appName}/log?since=${since}`);
  }

  updateApp(appName, attrs) {
    return this.invoke("PUT", `/apps/${appName}`, attrs);
  }

  deleteApp(appName) {
    return this.invoke("DELETE", `/apps/${appName}`);
  }

  getIntegrations(appName) {
    return this.invoke("GET", `/apps/${appName}/integrations`);
  }

  createIntegration(appName, service, config, comment) {
    return this.invoke("POST", `/apps/${appName}/integrations`,
      { service, config, comment });
  }

  updateIntegration(appName, service, config, comment) {
    return this.invoke("PUT", `/apps/${appName}/integrations/${service}`,
      { service, config, comment });
  }

  deleteIntegration(appName, service) {
    return this.invoke("GET", `/apps/${appName}/integrations/${service}`);
  }

  getDevices() {
    return this.invoke("GET", `/devices`);
  }

  getDevice(deviceName) {
    return this.invoke("GET", `/devices/${deviceName}`);
  }

  getDeviceLog(deviceName) {
    return this.invoke("GET", `/devices/${deviceName}/log`);
  }

  getDeviceStores(deviceName) {
    return this.invoke("GET", `/devices/${deviceName}/stores`);
  }

  updateDeviceStore(deviceName, key, value) {
    return this.invoke("PUT", `/devices/${deviceName}/stores/${key}`,
    { value });
  }

  associateDeviceToApp(deviceName, appName) {
    return this.invoke("PATCH", `/devices/${deviceName}`, { device: { app_name: appName } });
  }

  deleteDevice(deviceName) {
    return this.invoke("DELETE", `/devices/${deviceName}`);
  }

  getDeployments(appName) {
    return this.invoke("GET", `/apps/${appName}/deployments`);
  }

  getDeployment(appName, version) {
    return this.invoke("GET", `/apps/${appName}/deployments/${version}`);
  }

  deploy(appName, image, debug, comment, tag) {
    return this.invoke("POST", `/apps/${appName}/deployments`,
      { deployment: { image, debug, comment, tag } });
  }

  createApp(appName, api) {
    return this.invoke("POST", `/apps`, {
      app: { name: appName, api: api }
    });
  }

  getFiles(appName) {
    return this.invoke("GET", `/apps/${appName}/files`);
  }

  saveFile(appName, path, body) {
    return this.invoke("PUT", `/apps/${appName}/files/${path}`, { body });
  }

  getAppStores(appName) {
    return this.invoke("GET", `/apps/${appName}/stores`);
  }

  createAppStore(appName, key, type, value) {
    return this.invoke("POST", `/apps/${appName}/stores`, {
      store: { key, data_type: data_type, type, value }
    });
  }

  updateAppStore(appName, key, value) {
    return this.invoke("PUT", `/apps/${appName}/stores/${key}`,
      { store: { value } });
  }
}
