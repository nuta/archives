import * as FormData from "form-data";
import * as fs from "fs";
import * as util from "util";
import { loadCredentials, saveCredentials } from "./config";
import { FatalError } from "./types";
const fetch = require("node-fetch");

class API {
    public invoke(method: "GET" | "POST" | "PUT" | "DELETE" | "PATCH", path: string, body?: any): Promise<any> {
        const headers = {};

        return new Promise((resolve, reject) => {
            const credentials = loadCredentials();
            if (!credentials) {
                reject(new FatalError("login first"));
            }

            if (!(body instanceof FormData)) {
                body = JSON.stringify(body);
                Object.assign(headers, {
                    "Content-Type": "application/json",
                });
            }

            let status: number;
            fetch(`${credentials.url}/api/v1${path}`, {
                method,
                headers: Object.assign(headers, credentials),
                body,
            }).then((response: any) => {
                status = response.status;
                return (status === 204) ? Promise.resolve({}) : response.json();
            }).then((json: any) => {
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

    public login(url: string, username: string, password: string) {
        let status: number, headers: any;
        return fetch(`${url}/api/v1/auth/sign_in`, {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ username, password }),
        }).then((response: any) => {
            status = response.status;
            headers = response.headers;
            return response.json();
        }).then((json: any) => {
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

    public createApp(appName: string, api: string) {
        return this.invoke("POST", `/apps`, {
            app: { name: appName, api },
        });
    }

    public getFiles(appName: string) {
        return this.invoke("GET", `/apps/${appName}/files`);
    }

    public saveFile(appName: string, path: string, body: string) {
        return this.invoke("PUT", `/apps/${appName}/files/${path}`, { body });
    }

    public getApp(appName: string) {
        return this.invoke("GET", `/apps/${appName}`);
    }

    public editApp(appName: string, attrs: string) {
        return this.invoke("PUT", `/apps/${appName}`, attrs);
    }

    public getDeployments(appName: string) {
        return this.invoke("GET", `/apps/${appName}/deployments`);
    }

    public getDeployment(appName: string, version: string) {
        return this.invoke("GET", `/apps/${appName}/deployments/${version}`);
    }

    public deploy(appName: string, image: Buffer, debug?: Buffer, comment?: string, tag?: string) {
        const form = new FormData();
        form.append("deployment[deployed_at]", "client");
        form.append("deployment[image]", image);
        return this.invoke("POST", `/apps/${appName}/deployments`, form);
    }

    public deleteApp(appName: string) {
        return this.invoke("DELETE", `/apps/${appName}`);
    }

    public getDevices() {
        return this.invoke("GET", `/devices`);
    }

    public getDevice(deviceName: string) {
        return this.invoke("GET", `/devices/${deviceName}`);
    }

    public updateDevice(deviceName: string, attrs: any) {
        return this.invoke("PATCH", `/devices/${deviceName}`, { device: attrs });
    }

    public getDeviceLog(deviceName: string) {
        return this.invoke("GET", `/devices/${deviceName}/log`);
    }

    public getDeviceStores(deviceName: string) {
        return this.invoke("GET", `/devices/${deviceName}/stores`);
    }

    public setDeviceStore(deviceName: string, key: string, value: string) {
        return this.invoke("PUT", `/devices/${deviceName}/stores/${key}`, { value });
    }

    public registerDevice(name: string, deviceType: string, tag?: string) {
        return this.invoke("POST", "/devices", { device: { name, device_type: deviceType, tag } });
    }

    public deleteDevice(deviceName: string) {
        return this.invoke("DELETE", `/devices/${deviceName}`);
    }

    public getAppStores(appName: string) {
        return this.invoke("GET", `/apps/${appName}/stores`);
    }

    public setAppStore(appName: string, key: string, value: string) {
        return this.invoke("PUT", `/apps/${appName}/stores/${key}`, { value });
    }

    public downloadPlugin(name: string): Promise<Buffer> {
        let repo: string;
        if (name.includes("/")) {
            // A third-party plugin on GitHub.
            repo = name;
            name = name.split("/").pop() as string;
        } else {
            repo = "_/_";
        }

        return new Promise((resolve, reject) => {
            fetch(`${this.serverURL}/api/v1/plugins/${repo}/${name}`, {
                headers: loadCredentials(),
            }).then((response: any) => {
                response.buffer().then(resolve, reject);
            });
        });
    }

    public getAppLog(appName: string, since?: Date): Promise<{lines: [string]}> {
        const unixtime = since ? Math.floor(since.getTime() / 1000) : 0;
        return this.invoke("GET", `/apps/${appName}/log?since=${unixtime}`);
    }

    public async streamAppLog(appName: string, callback: (lines: string[]) => void) {
        callback((await this.getAppLog(appName)).lines);

        let lastFetchedAt: Date | null = null;
        setInterval(async () => {
            callback((await this.getAppLog(appName, lastFetchedAt || undefined)).lines);
            lastFetchedAt = new Date();
        }, 5000);
    }

    public getOSReleases() {
        return this.invoke("GET", "/os/releases");
    }

    public invokeCommand(deviceName: string, command: string, arg: string) {
        return this.invoke("POST", `/devices/${deviceName}/commands`, { command, arg });
    }

    public getCommandResults(deviceName: string) {
        return this.invoke("GET", `/devices/${deviceName}/commands`);
    }
}

export const api = new API();
