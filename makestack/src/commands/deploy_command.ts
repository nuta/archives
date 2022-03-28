import {
    Args,
    Command,
    Opts,
    CLOUD_OPTS,
    BOARD_OPTS,
    APP_OPTS,
    BUILD_OPTS,
} from "./command";
import { buildApp } from "../firmware";
import { logger } from "../logger";
import { DeployOptions } from "../clouds";
import { BuildOptions } from "../boards";

export class DeployCommand extends Command {
    public static command = "deploy";
    public static desc = "Deploy the app.";
    public static args = [];
    public static opts = [
        ...APP_OPTS,
        ...BUILD_OPTS,
        ...BOARD_OPTS,
        ...CLOUD_OPTS
    ];

    public async run(args: Args, opts: Opts) {
        logger.progress("Building the firmware...");
        await buildApp(opts.board, opts.appDir, opts as BuildOptions);
        await opts.cloud.deploy(
            opts.appDir,
            opts.board.getFirmwarePath(),
            opts as DeployOptions
        );
    }
}
