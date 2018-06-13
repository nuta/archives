import { Args, CommandBase, Opts } from "../cli";
import { logger } from "../logger";
import { getSdkInstance } from "../platform";

export class Command extends CommandBase {
    public static command = "command";
    public static desc = "";
    public static args = [
        { name: "command", desc: "The command name." },
        { name: "arg", desc: "The command argument." },
    ];
    public static opts = [
        { name: "--device", desc: "The target device.", required: true },
        // TODO:
        { name: "--firebase-project", desc: "The Firebase project name." },
    ];

    public async run(args: Args, opts: Opts) {
        // FIXME:
        opts.platform = "local";
        await getSdkInstance(opts.platform).command(opts.device, args.command, args.arg);
    }
}
