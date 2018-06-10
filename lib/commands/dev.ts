const chalk = require("chalk");
import * as fs from "fs";
import * as path from "path";
import * as child_process from "child_process";
import { App } from "../app";
import { Args, Opts, Command } from "../cli";
import { board } from "../boards";

export default class DevCommand extends Command {
    static command = "dev";
    static desc = "";
    static args = [];
    static opts = [
        { name: '--app-dir', desc: 'The app directory.', default: process.cwd() }
    ];

    async run(args: Args, opts: Opts, logger: Logger) {
        logger.info("starting dev")

        logger.info("Building...")
        await board.build(opts.appDir);

        const localRuntimePath = path.resolve(__dirname, '../platform/local/start');
        const forkOptions = {
            cwd: opts.appDir,
            env: Object.assign({}, process.env, {
                // This points to node_modules or the repo directory.
                NODE_PATH: path.resolve(__dirname, "../../../.."),
                MAKESTACK_PLATFORM: "local"
            })
        }

        let app = child_process.fork(localRuntimePath, [], forkOptions);
        process.on("exit", () => {
            app.kill()
        })

        const reload = async (filepath: string) => {
            logger.info(`==> change detected: ${filepath}`)
            logger.info("Building...")
            await board.build(opts.appDir);

            logger.info("killing...");
            app.kill();

            app.on("exit", () => {
                logger.info("reloading...");
                app = child_process.fork(localRuntimePath, [], forkOptions);
            })
        }

        fs.watch(path.resolve(opts.appDir, "server.js"), (_: string, filepath: string) => reload(filepath))
        fs.watch(path.resolve(opts.appDir, "device.cc"), (_: string, filepath: string) => reload(filepath))

        logger.info("Press Ctrl-C to stop")
    }
}
