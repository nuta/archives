import * as FormData from "form-data";
import * as fs from "fs";
import * as fetch from "node-fetch";
import * as util from "util";
import { loadCredentials, saveCredentials } from "./config";

class API {
  public invoke(method: "GET" | "POST" | "PUT" | "DELETE" | "PATCH", path: string, body?: any): Promise<any> {
    const headers = {};

    return new Promise((resolve, reject) => {
      const credentials = loadCredentials();
      if (!credentials) {
        reject(new Error("login first"));
      }

      if (!(body instanceof FormData)) {
        body = JSON.stringify(body);
        Object.assign(headers, {
          "Content-Type": "application/json",
        });
      }

      let status;
      fetch(`${credentials.url}/api/v1${path}`, {
        method,
        headers: Object.assign(headers, credentials),
        body,
      }).then((response) => {
        status = response.status;
        return (status === 204) ? Promise.resolve({}) : response.json();
      }).then((json) => {
        if (!(status >= 200 && status <= 300)) {
          const msg = util.inspect(json);
          reject(Error(`Error: server returned ${status}: \`${msg}'`));
        }

        resolve(json);
      });
    });
  }

  get serverURL() {
    return loadCredentials().url;
  }

  public login(url, username, password) {
    let status, headers;
    return fetch(`${url}/api/v1/auth/sign_in`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ username, password }),
    }).then((response) => {
      status = response.status;
      headers = response.headers;
      return response.json();
    }).then((json) => {
      if (status !== 200) {
        throw new Error(`Error: failed to login: \`${json.errors}'`);
      }

      saveCredentials({
        url,
        username,
        "email": json.data.email,
        "uid": headers.get("uid"),
        "access-token": headers.get("access-token"),
        "access-token-secret": headers.get("access-token-secret"),
      });
    });
  }

  public getApps() {
    return this.invoke("GET", `/apps`);
  }

  public createApp(appName, api) {
    return this.invoke("POST", `/apps`, {
      app: { name: appName, api },
    });
  }

  public getFiles(appName) {
    return this.invoke("GET", `/apps/${appName}/files`);
  }

  public saveFile(appName, path, body) {
    return this.invoke("PUT", `/apps/${appName}/files/${path}`, { body });
  }

  public getApp(appName) {
    return this.invoke("GET", `/apps/${appName}`);
  }

  public editApp(appName, attrs) {
    return this.invoke("PUT", `/apps/${appName}`, attrs);
  }

  public getDeployments(appName) {
    return this.invoke("GET", `/apps/${appName}/deployments`);
  }

  public getDeployment(appName, version) {
    return this.invoke("GET", `/apps/${appName}/deployments/${version}`);
  }

  public deploy(appName: string, image: Buffer, debug?: Buffer, comment?: string, tag?: string) {
    const form = new FormData();
    form.append("deployment[deployed_at]", "client");
    form.append("deployment[image]", image);
    return this.invoke("POST", `/apps/${appName}/deployments`, form);
  }

  public deleteApp(appName) {
    return this.invoke("DELETE", `/apps/${appName}`);
  }

  public getDevices() {
    return this.invoke("GET", `/devices`);
  }

  public getDevice(deviceName) {
    return this.invoke("GET", `/devices/${deviceName}`);
  }

  public updateDevice(deviceName, attrs) {
    return this.invoke("PATCH", `/devices/${deviceName}`, { device: attrs });
  }

  public getDeviceLog(deviceName) {
    return this.invoke("GET", `/devices/${deviceName}/log`);
  }

  public getDeviceStores(deviceName) {
    return this.invoke("GET", `/devices/${deviceName}/stores`);
  }

  public setDeviceStore(deviceName, key, value) {
    return this.invoke("PUT", `/devices/${deviceName}/stores/${key}`, { value });
  }

  public registerDevice(name, deviceType, tag?) {
    return this.invoke("POST", "/devices", { device: { name, device_type: deviceType, tag } });
  }

  public deleteDevice(deviceName) {
    return this.invoke("DELETE", `/devices/${deviceName}`);
  }

  public getAppStores(appName) {
    return this.invoke("GET", `/apps/${appName}/stores`);
  }

  public setAppStore(appName, key, value) {
    return this.invoke("PUT", `/apps/${appName}/stores/${key}`, { value });
  }

  public downloadPlugin(name) {
    let repo;
    if (name.includes("/")) {
      // A third-party plugin on GitHub.
      repo = name;
      name = "nodejs-" + name.split("/").pop();
    } else {
      repo = "_/_";
    }

    return new Promise((resolve, reject) => {
      fetch(`${this.serverURL}/api/v1/plugins/${repo}/${name}`, {
        headers: loadCredentials(),
      }).then((response) => {
        response.buffer().then(resolve, reject);
      });
    });
  }

  public getAppLog(appName, since?): Promise<{lines: [string]}> {
    const unixtime = since ? Math.floor(since.getTime() / 1000) : 0;
    return this.invoke("GET", `/apps/${appName}/log?since=${unixtime}`);
  }

  public async streamAppLog(appName, callback) {
    callback((await this.getAppLog(appName)).lines);

    let lastFetchedAt = null;
    setInterval(async () => {
      callback((await this.getAppLog(appName, lastFetchedAt)).lines);
      lastFetchedAt = new Date();
    }, 5000);
  }

  public getOSReleases() {
    return this.invoke("GET", "/os/releases");
  }

  public invokeCommand(deviceName, command, arg) {
    return this.invoke("POST", `/devices/${deviceName}/commands`, { command, arg });
  }

  public getCommandResults(deviceName) {
    return this.invoke("GET", `/devices/${deviceName}/commands`);
  }
}

export const api = new API();
