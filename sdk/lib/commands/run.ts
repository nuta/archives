import * as os from "os";
import * as path from "path";

export async function main(args, opts, logger) {
  if (opts.adapter === "http" && !opts.server) {
    throw new Error("--server is missing");
  }

  const Supervisor = require("../../../supervisor");
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
