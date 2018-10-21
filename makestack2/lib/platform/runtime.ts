import * as express from "express";
import * as fs from "fs-extra";
import * as path from "path";
import { Device } from "../device";
import { getFirmwareVersion } from "../firmware";
import { instantiatePlugins } from "../plugins";
import { endpoints } from "../server";
import * as telemata from "../telemata";
import { DeviceData } from "../types";

function basicAuthMiddleware(req: express.Request, res: express.Response, next: Function) {
    const env = process.env.BASIC_AUTH;
    if (!env) {
        // Basic authentication is disabled.
        next();
        return;
    }

    const cred = env.split(":", 2);
    const requestsFromDevices = [
        "/makestack/telemata",
        "/makestack/firmware",
    ];

    if (requestsFromDevices.includes(req.path)) {
        // Currently no authentication mechanism are implementd.
        next();
    }

    const header = req.headers.authorization;
    if (!header) {
        res.status(401).header("WWW-Authenticate", 'Basic realm=""').end();
        return;
    }

    const base64Data = header.split("Basic ")[1];
    if (!base64Data) {
        res.status(401).header("WWW-Authenticate", 'Basic realm=""').end();
        return;
    }

    const client = Buffer.from(base64Data, "base64").toString();
    const [username, password] = client.split(":", 2);
    if (username !== cred[0] || password !== cred[1]) {
        res.status(401).header("WWW-Authenticate", 'Basic realm=""').end();
        return;
    }

    next();
}

function rawBodyMiddleware(req: any, res: express.Response, next: Function) {
    req.rawBody = Buffer.alloc(0);

    req.on("data", function(chunk: Buffer) {
        req.rawBody = Buffer.concat([req.rawBody, chunk]);
    });

    req.on("end", function() {
        next();
    });
}

export abstract class PlatformRuntime {
    public httpServer: express.Express;
    private appDir: string;

    public abstract async getDeviceData(deviceName: string): Promise<DeviceData>;
    public abstract async setDeviceData(deviceName: string, data: DeviceData): Promise<void>;

    constructor() {
        this.appDir = process.env.APP_DIR || process.cwd();
        this.httpServer = express();
        this.httpServer.use(basicAuthMiddleware);
        this.httpServer.use(express.static("public"));
        this.httpServer.use(rawBodyMiddleware);
    }

    protected doInit() {
        this.loadPlugins();
        this.loadAppServer();
    }

    private loadAppServer() {
        require(path.resolve(this.appDir, "./server"));
        for (const [name, callback] of Object.entries(endpoints)) {
            // TODO: support other methods
            this.httpServer.get(name, callback);
        }
    }

    private loadPlugins() {
        const config = fs.readJsonSync(path.join(this.appDir, "package.json")).makestack;
        if (!config) {
            throw new Error("Specify makestack.plugins and makestack.devPlugins in package.json");
        }

        let requiredPlugins = config.plugins || [];
        if (!process.env.MAKESTACK_PRODUCTION) {
            requiredPlugins = requiredPlugins.concat(config.devPlugins);
        }

        const plugins = instantiatePlugins(requiredPlugins);

        for (const plugin of plugins) {
            if (plugin.server) {
                plugin.server(this.httpServer);
            }

            if (plugin.receivePayload) {
                plugin.receivePayload(async (payload: Buffer) => {
                    const device = await telemata.process(payload);
                    const commands = device.dequeuePendingCommands();

                    device.save();
                    if (!device.state) {
                        return telemata.serialize({ commands });
                    }

                    // TODO: remove this
                    const firmwarePath = path.resolve(`${device.state.board}.firmware`);
                    const firmwareImage = fs.readFileSync(firmwarePath);
                    const appVersion = getFirmwareVersion(firmwareImage);

                    if (appVersion !== device.state.version) {
                        const update = { type: "bulk", version: appVersion };
                        return telemata.serialize({ update, commands } as any);
                    } else {
                        return telemata.serialize({ commands });
                    }
                });
            }
        }
    }
}
