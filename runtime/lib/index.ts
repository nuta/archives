import * as path from "path";
import { AppAPI } from "./api/app";
import { publish } from "./api/event";
import { error, println } from "./api/logging";
import { SerialAPI } from "./api/serial";
import { ConfigAPI } from "./api/config";
import { SubProcessAPI } from "./api/subprocess";
import { TimerAPI } from "./api/timer";
import { logger } from "./logger";

export const builtins = {
    Timer: new TimerAPI(),
    Config: new ConfigAPI(),
    App: new AppAPI(),
    SubProcess: new SubProcessAPI(),
    Serial: SerialAPI,
    println,
    error,
    publish,
};

if (process.env.MAKESTACK_DEVICE_TYPE) {
    const deviceType = process.env.MAKESTACK_DEVICE_TYPE;
    const device = require(`./devices/${deviceType}`);
    Object.assign(builtins, device.initialize());
}

export function start(appDir: string) {
    process.on("unhandledRejection", (reason, p) => {
        console.log("runtime: unhandled rejection:\n", reason, "\n\n", p);
        console.log("runtime: exiting...");
        process.exit(1);
    });

    Object.assign(global, builtins);

    process.on("message", (data) => {
        switch (data.type) {
            case "initialize":
            logger.info(`initialize message: configs=${JSON.stringify(data.configs)}`);
            builtins.Config.update(data.configs);

            // Start the app.
            logger.info("staring the app");
            process.chdir(appDir);
            require(path.resolve(appDir, "app"));
            logger.info("started the app");
            break;

            case "configs":
            logger.info(`configs message: configs=${JSON.stringify(data.configs)}`);
            builtins.Config.update(data.configs);
            break;

            default:
            logger.info("unknown ipc message: ", data);
        }
    });

    logger.info("waiting for `initialize' message from Supervisor...");
}
