import * as fs from "fs-extra";
import * as net from "net";
import { Device } from "../../device";
import { logger } from "../../logger";
import { DeviceData } from "../../types";
import { PlatformRuntime } from "../runtime";

export interface LocalRequest {
    type: string;
    command?: {
        name: string;
        device: string;
        arg: string;
    };
}

export class LocalPlatformRuntime extends PlatformRuntime {
    private devices: { [deviceName: string]: DeviceData };

    constructor() {
        super();
        this.devices = {};
    }

    public async getDeviceData(deviceName: string): Promise<DeviceData> {
        return this.devices[deviceName] || {};
    }

    public async setDeviceData(deviceName: string, data: DeviceData) {
        this.devices[deviceName] = data;
    }

    private async handleLocalRequest(req: LocalRequest) {
        switch (req.type) {
            case "command":
                if (!req.command) {
                    return { result: "error" };
                }

                const device = await Device.getByName(req.command.device);
                device.command(req.command.name, req.command.arg);
                device.save();
                return { result: "success" };
            default:
                return { result: "error" };
        }
    }

    public init() {
        this.doInit();
    }

    public start(host: string, port: number) {
        this.startLocalServer();
        this.httpServer.listen(port, host);
    }

    // Listen on an UNIX domain socket named "dev.sock" to accept requests
    // from the developer such as `makestack command'.
    private startLocalServer() {
        const sock = net.createServer();
        sock.on("connection", (client) => {
            client.on("data", async (data) => {
                let req;
                try {
                    req = JSON.parse(data.toString("utf-8"));
                } catch (e) {
                    logger.error(e);
                    logger.error(e.stack);
                    client.write(JSON.stringify({ result: "error" }));
                }
                client.write(JSON.stringify(await this.handleLocalRequest(req)));
            });
        });

        fs.removeSync("dev.sock");
        sock.listen("dev.sock");
    }
}
