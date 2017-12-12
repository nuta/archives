import * as os from "os";
import * as path from "path";
import { api } from "../api";
import { Supervisor } from "@makestack/supervisor";
import { loadMocks, updateMocks } from "../config";

export function create(args: any, opts: any, logger: any) {
    const deviceName = args.name;
    api.registerDevice(deviceName, "sdk").then((device) => {
        updateMocks({ [deviceName]: device });
    }).catch((e) => {
        logger.error("failed to create a mock device", e);
    });
}

export function run(args: any, opts: any, logger: any) {
    const mock = loadMocks()[args.name];

    const supervisor = new Supervisor({
        mode: 'production',
        appDir: path.resolve(os.homedir(), ".makestack/mock-app"),
        adapter: {
            name: opts.adapter,
            url: api.serverURL,
        },
        osType: "sdk",
        osVersion: "dev",
        deviceId: mock.device_id,
        deviceSecret: mock.device_secret,
        heartbeatInterval: 2
    });

    supervisor.start();
}
