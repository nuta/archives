let fs = require("fs");
let path = require("path");
let fetch = require("node-fetch");
let config = require("./config");

module.exports = new class {
  constructor() {
    this.credentialsPath = path.join(config.configDir, "credentials.json");
    try {
      this.credentials = JSON.parse(fs.readFileSync(this.credentialsPath));
      this.serverUrl = this.credentials.serverUrl;
    } catch (e) {
      try {
        fs.mkdirSync(path.dirname(this.credentialsPath));
      } catch (e) {
        // ignore
      }

      fs.writeFileSync(this.credentialsPath, "{}");
    }
  }

  invoke(method, path, params, requiresCredentials=true) {
    let reqHeaders = Object.assign({
      "Content-Type": "application/json"
    }, this.credentials);

    return new Promise((resolve, reject) => {
      let status;
      let headers;
      fetch(`${this.serverUrl}/api/v1${path}`, {
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

  login(serverUrl, username, password) {
    this.serverUrl = serverUrl;
    return this.invoke("POST", "/auth/sign_in", {
      username: username,
      password: password
    }, false).then(r => {
      this.credentials = {
        serverUrl: serverUrl,
        username: username,
        email: r.json["data"]["email"],
        uid: r.headers.get("uid"),
        "access-token": r.headers.get("access-token"),
        "access-token-secret": r.headers.get("access-token-secret")
      };
  
      fs.writeFileSync(this.credentialsPath, JSON.stringify(this.credentials));
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

  sendHeartbeat(deviceId, state, appVersion, log) {
    let deviceIdBuffer = Buffer.from(deviceId);
    let logBuffer = Buffer.from(log);
    if (logBuffer.length > 0xffff) {
      // TODO
      throw "Too long log.";
    }

    let packetLen = 8 /* header */ + 4 * 2 + 3 + 4 + deviceIdBuffer.length + logBuffer.length;
    let packet = new Buffer.alloc(packetLen);

    // Headers
    packet.writeUInt8(1, 0); // version
    packet.writeUInt8(0, 1); // reserved
    let offset = 2;

    // deviceId
    packet.writeUInt8(0x8a, offset); // type
    packet.writeUInt8(0x00, offset + 1); // reserved
    packet.writeUInt16BE(deviceIdBuffer.length, offset + 2);
    offset += 4;
    for (var i=0; i < deviceIdBuffer.length; i++) {
      packet.writeUInt8(deviceIdBuffer[i], offset);
      offset++;
    }

    // deviceInfo
    let states = { new: 1, booting: 2, ready: 3, running: 4, down: 5, reboot: 6, relaunch: 7 }
    if (!states[state]) {
      throw `Invalid device state: \`${state}'`;
    }

    packet.writeUInt8(0x0b, offset); // type
    packet.writeUInt8(0x01, offset + 1); // length
    packet.writeUInt8(states[state], offset + 2); // id
    offset += 3;

    // appVersion
    packet.writeUInt8(0x0d, offset); // id
    packet.writeUInt8(0x02, offset + 1); // length
    packet.writeUInt16BE(appVersion, offset + 2);
    offset += 4;
    
    // log
    packet.writeUInt8(0x8c, offset); // type
    packet.writeUInt8(0x00, offset + 1); // reserved
    packet.writeUInt16BE(logBuffer.length, offset + 2);
    offset += 4;
    for (var i=0; i < logBuffer.length; i++) {
      packet.writeUInt8(logBuffer[i], offset);
      offset++;
    }

    return new Promise((resolve, reject) => {
      fetch(`${this.serverUrl}/api/v1/heartbeat`, {
        method: "POST",
        body: packet
      }).then(response => {
        if (200 <= response.status && response.status <= 299)
          resolve(response);
        else
          reject(response);
      });
    });
  }
}
