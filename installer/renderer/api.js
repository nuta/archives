const fs = require("fs");
const path = require("path");
const fetch = require("whatwg-fetch");

const CONFIG_DIR = `${process.env.HOME}/.makestack`;

config = new class {
    get server()    { return this.load("server"); }
    set server(val) { return this.save("server", val); }
    get mocks()    { return this.load("mocks"); }
    set mocks(val) { return this.save("mocks", val); }
    get credentials()    { return this.load("credentials"); }
    set credentials(val) { return this.save("credentials", val); }

    load(name) {
      let json
      try {
        json = fs.readFileSync(`${CONFIG_DIR}/${name}.json`)
      } catch (e) {
        return null
      }

      return JSON.parse(json)
    }
    
    save(name, data) {
        const path = `${CONFIG_DIR}/${name}.json`;
        try {
            fs.mkdirSync(path.dirname(path));
        } catch (e) {
            // ignore
        }
        
        fs.writeFileSync(path, JSON.stringify(data));
    }
};

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

  get serverURL() {
    return config.server.url
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

  isLoggedIn() {
    return config.credentials !== null
  }

  updateDeviceStore(deviceName, key, value) {
    return this.invoke("PUT", `/devices/${deviceName}/stores/${key}`, { value });
  }
  
  registerDevice(name, deviceType, tag) {
    return this.invoke("POST", "/devices", { device: { name, device_type: deviceType, tag } });
  }
}
