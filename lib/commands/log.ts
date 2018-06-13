import { Args, CommandBase, Opts, constructEnvOption } from "../cli";
import { logger } from "../logger";
import { getSdkInstance } from "../platform";
import { loadConfig } from "../helpers";

export class Command extends CommandBase {
    public static command = "log";
    public static desc = "";
    public static args = [];
    public static opts = [
        constructEnvOption("development"),
    ];

    public async run(args: Args, opts: Opts) {
        const config = loadConfig(opts.appDir, opts.env);
        logger.progress(`Reading logs from ${config.platform}`);
        getSdkInstance(config.platform).viewLog(opts);
    }
}
