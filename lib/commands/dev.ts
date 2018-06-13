const chalk = require("chalk");
import * as child_process from "child_process";
import * as fs from "fs";
import * as path from "path";
import { App } from "../app";
import { board } from "../boards";
import { Args, CommandBase, Opts } from "../cli";
import { downloadRepo, generateRandomVersion } from "../helpers";
import { logger } from "../logger";

export class Command extends CommandBase {
    public static command = "dev";
    public static desc = "";
    public static args = [];
    public static opts = [
        { name: "--app-dir", desc: "The app directory.", default: process.cwd() },
        { name: "--repo-dir", desc: "The file path to the seiyanuta/makestack repo." },
    ];

    public async run(args: Args, opts: Opts) {
        const repoDir = opts.repoDir || downloadRepo(opts.appDir);
        const localRuntimePath = path.resolve(__dirname, "../platform/local/start");

        const forkOptions = {
            cwd: opts.appDir,
            env: Object.assign({}, process.env, {
                // This points to node_modules or the repo directory.
                NODE_PATH: path.resolve(__dirname, "../../../.."),
                MAKESTACK_PLATFORM: "local",
            }),
        };

        let app: child_process.ChildProcess;
        process.on("exit", () => {
            app.kill();
        });

        try {

            logger.progress("Building the firmware");
            await board.build(false, generateRandomVersion(), repoDir, opts.appDir);
            logger.progress("Launching the server");
            app = child_process.fork(localRuntimePath, [], forkOptions);
        } catch (e) {
            logger.error(e.message);
        }

        const reload = async (filepath: string) => {
            logger.progress(`Change detected: ${filepath}`);
            logger.progress("Building the firmware");
            try {
                await board.build(false, generateRandomVersion(), repoDir, opts.appDir);
            } catch (e) {
                logger.error(e.message);
                return;
            }

            logger.progress("Restarting the server");
            app.kill();
            app.on("exit", () => {
                app = child_process.fork(localRuntimePath, [], forkOptions);
            });
        };

        fs.watch(path.resolve(opts.appDir, "server.js"), (_: string, filepath: string) => reload(filepath));
        fs.watch(path.resolve(opts.appDir, "device.cc"), (_: string, filepath: string) => reload(filepath));

        logger.info("*** Watching the changes to source code...");
        logger.info("*** Press Ctrl-C to quit");
    }
}
