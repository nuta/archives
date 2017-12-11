import * as os from "os";
import * as path from "path";
import { Supervisor } from "@makestack/supervisor";
import { FatalError } from "../types";

export async function main(args: any, opts: any, logger: any) {
    if (opts.adapter === "http" && !opts.server) {
        throw new FatalError("Specify `server' option.");
    }

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
