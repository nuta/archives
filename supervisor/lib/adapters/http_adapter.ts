import http = require("http");
import https = require("https");
import { parse as parseURL } from "url";
import * as logger from "../logger";
import { AdapterBase } from "./adapter_base";

function request(method: string, url: string, body?: Buffer): Promise<Buffer> {
    return new Promise((resolve, reject) => {
        const send: typeof http.request = (url.startsWith("https://") ? https : http).request;

        const options = Object.assign({}, parseURL(url), { method });
        const req = send(options);

        if (body) {
            req.write(body);
        }

        req.end();

        let responseBody = Buffer.alloc(0);

        req.on("error", (err) => {
            reject(new Error(`failed to perform a http request: ${err.message}`));
        });

        req.once("response", (res: http.IncomingMessage) => {
            res.on("data", (chunk: Buffer) => {
                responseBody = Buffer.concat([responseBody, chunk]);
            });

            res.on("end", () => {
                if (res.statusCode === 200) {
                    resolve(responseBody);
                } else {
                    reject(new Error(`server returned ${res.statusMessage}`));
                }
            });
        });
    });
}

export class HTTPAdapter extends AdapterBase {
    public osType: string;
    public deviceType: string;
    public deviceId: string;
    public serverURL: string;

    constructor(osType: string, deviceType: string, deviceId: string, serverURL: string) {
        super();

        this.osType = osType;
        this.deviceType = deviceType;
        this.deviceId = deviceId;
        this.serverURL = serverURL;
    }

    public async connect() {
        // Nothing to do.
    }

    public send(payload: Buffer) {
        return request("POST", `${this.serverURL}/api/v1/smms`, payload)
            .then(this.onReceiveCallback)
            .catch(logger.error);
    }

    public getAppImage(version: number) {
        const url = `${this.serverURL}/api/v1/images/app/${this.deviceId}/${version}`;
        return request("GET", url);
    }

    public getOSImage(version: number) {
        const url = `${this.serverURL}/api/v1/images/os/${this.deviceId}/${version}/${this.osType}/${this.deviceType}`;
        return request("GET", url);
    }
}
