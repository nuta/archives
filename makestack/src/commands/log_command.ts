import {
    Args,
    Command,
    Opts,
    CLOUD_OPTS,
    APP_OPTS,
} from "./command";
import { exec } from "../helpers";
import { logger } from "../logger";
import { DeployOptions } from "../clouds";

export class LogCommand extends Command {
    public static command = "log";
    public static desc = "Read logs from the deployed app.";
    public static args = [];
    public static opts = [
        ...APP_OPTS,
        ...CLOUD_OPTS,
    ];

    public async run(args: Args, opts: Opts) {
        logger.progress("Connecting...");
        opts.cloud.log(opts.appDir, opts as DeployOptions);
    }
}
