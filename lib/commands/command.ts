import { Args, CommandBase, Opts, constructEnvOption } from "../cli";
import { logger } from "../logger";
import { getSdkInstance } from "../platform";
import { loadConfig } from "../helpers";

export class Command extends CommandBase {
    public static command = "command";
    public static desc = "";
    public static args = [
        { name: "command", desc: "The command name." },
        { name: "arg", desc: "The command argument." },
    ];
    public static opts = [
        { name: "--device", desc: "The target device.", required: true },
        constructEnvOption("development"),
    ];

    public async run(args: Args, opts: Opts) {
        const config = loadConfig(opts.appDir, opts.env);
        await getSdkInstance(config.platform).command(opts.device, args.command, args.arg);
    }
}
