import * as path from "path";
import { scaffold } from "../scaffold";
import { Args, CommandBase, Opts } from "../cli";


export class Command extends CommandBase {
    public static command = "new";
    public static desc = "Create a new app.";
    public static args = [
        { name: "dir", desc: "The directory." },
    ];
    public static opts = [
        {
            name: "--frontend",
            desc: "The frontend framework.",
            default: "static",
            validators: ["static"],
        },
    ];

    public async run(args: Args, opts: Opts) {
        scaffold({
            dir: args.dir,
            name: path.basename(args.dir),
            frontend: opts.frontend,
        });
    }
}
