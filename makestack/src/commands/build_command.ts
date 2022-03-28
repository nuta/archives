import {
    Args,
    Command,
    Opts,
    BUILD_OPTS,
} from "./command";
import { buildApp } from "../firmware";
import { logger } from "../logger";
import { BuildOptions } from "../boards";

export class BuildCommand extends Command {
    public static command = "build";
    public static desc = "Build the firmware.";
    public static args = [];
    public static opts = [ ...BUILD_OPTS ];
    public static watchMode = true;

    public async run(_args: Args, opts: Opts) {
        logger.progress("Building the firmware...");
        await buildApp(opts.board, opts.appDir, opts as BuildOptions);
        logger.success("Done");
    }
}
