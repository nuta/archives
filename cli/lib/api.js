let fs = require("fs");
let path = require("path");
let { fetch } = require("hyperutils");
let config = require("./config");

module.exports = new class {
  invoke(method, path, params, requiresCredentials=true) {
    let reqHeaders = Object.assign({
      "Content-Type": "application/json"
    }, config.credentials);

    return new Promise((resolve, reject) => {
      let status;
      let headers;
      fetch(`${config.server.url}/api/v1${path}`, {
        method: method,
        headers: reqHeaders,
        body: JSON.stringify(params)
      }).then(response => {
        status = response.status;
        headers = response.headers;
        return response;
      }).then(response => {
        return response.json();
      }).then(json => {
        if (200 <= status && status <= 299)
          resolve({status, headers, json });
        else
          reject({status, headers, json });
      });
    });
  }

  logout() {
    fs.unlinkSync(this.credentialsPath);
  }

  login(url, username, password) {
    config.server = { url };
    return this.invoke("POST", "/auth/sign_in", {
      username: username,
      password: password
    }, false).then(r => {
      config.credentials = {
        username: username,
        email: r.json["data"]["email"],
        uid: r.headers.get("uid"),
        "access-token": r.headers.get("access-token"),
        "access-token-secret": r.headers.get("access-token-secret")
      };
    });
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
      redirectUrl: "/home"
    }, false);
  }

  getApps() {
    return this.invoke("GET", `/apps`);
  }

  getApp(appName) {
    return this.invoke("GET", `/apps/${appName}`);
  }

  updateApp(appName, attrs) {
    return this.invoke("PUT", `/apps/${appName}`, attrs);
  }

  deleteApp(appName) {
    return this.invoke("DELETE", `/apps/${appName}`);
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
    return this.invoke("PUT", `/devices/${deviceName}/stores/${key}`, { value });
  }
  
  registerDevice(name, deviceType, tag) {
    return this.invoke("POST", "/devices", { device: { name, deviceType, tag } });
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

  updateAppStore(appName, key, value) {
    return this.invoke("PUT", `/apps/${appName}/stores/${key}`, { value });
  }
}
