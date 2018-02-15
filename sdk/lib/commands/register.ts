import * as os from "os";
import * as path from "path";
import { api } from "../api";
import { Supervisor } from "@makestack/supervisor";
import { loadDeviceConfig, saveDeviceConfig } from "../config";

export function main(args: any, opts: any, logger: any) {
    const deviceName = args.name;
    api.registerDevice(deviceName, "sdk", opts.app).then((device) => {
        saveDeviceConfig(Object.assign(device, {
            serverURL: api.serverURL
        }));
    }).catch(e => {
        logger.error("failed to create a device", e);
    });
}

