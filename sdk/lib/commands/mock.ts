import * as os from "os";
import * as path from "path";
import { api } from "../api";
import { loadMocks, updateMocks } from "../config";

export function create(args, opts, logger) {
    const deviceName = args.name;
    api.registerDevice(deviceName, "sdk", null).then((device) => {
        updateMocks({ [deviceName]: device });
    }).catch((e) => {
        logger.error("failed to create a mock device", e);
    });
}

export function run(args, opts, logger) {
    const mock = loadMocks()[args.name];

    const Supervisor = require("../../supervisor");
    const supervisor = new Supervisor({
        appDir: path.resolve(os.homedir(), ".makestack/mock-app"),
        adapter: {
            name: opts.adapter,
            url: api.serverURL,
        },
        osType: "sdk",
        deviceType: mock.device_type,
        deviceId: mock.device_id,
        deviceSecret: mock.device_secret,
        debugMode: true,
        osVersion: null,
        heartbeatInterval: 2,
    });

    supervisor.start();
}
