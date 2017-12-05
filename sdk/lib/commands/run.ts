import * as os from "os";
import * as path from "path";
import { FatalError } from "../types";

export async function main(args, opts, logger) {
    if (opts.adapter === "http" && !opts.server) {
        throw new FatalError("Specify `server' option.");
    }

    const { Supervisor } = require(path.resolve(__dirname, "../../supervisor"));
    const supervisor = new Supervisor({
        appDir: path.resolve(os.homedir(), ".makestack/app"),
        adapter: {
            name: opts.adapter,
            url: opts.server,
        },
        osType: "sdk",
        deviceType: "sdk",
        deviceId: opts.deviceId,
        deviceSecret: opts.deviceSecret,
        debugMode: true,
        osVersion: null,
        heartbeatInterval: opts.heartbeatInterval,
        runtimeModulePath: path.resolve(__dirname, "../../../runtime"),
    });

    supervisor.start();
}
