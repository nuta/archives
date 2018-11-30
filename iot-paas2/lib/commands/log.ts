import { Args, CommandBase, constructEnvOption, Opts } from "../cli";
import { loadConfig } from "../helpers";
import { logger } from "../logger";
import { getSdkInstance } from "../platform";

export class Command extends CommandBase {
    public static command = "log";
    public static desc = "";
    public static args = [];
    public static opts = [
        { name: "--app-dir", desc: "The app directory.", default: process.cwd() },
        constructEnvOption("development"),
    ];

    public async run(args: Args, opts: Opts) {
        const config = loadConfig(opts.appDir, opts.env);
        logger.progress(`Reading logs from ${config.platform}`);
        const sdk = getSdkInstance(config.platform);
        sdk.viewLog(opts);
    }
}
