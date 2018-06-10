const chalk = require("chalk");
import * as fs from "fs";
import * as path from "path";
import * as child_process from "child_process";
import { Args, Opts, Command } from "../cli";
import { board } from "../boards";


export default class DeployCommand extends Command {
    static command = "deploy";
    static desc = "";
    static args = [];
    static opts = [
        { name: '--app-dir', desc: 'The app directory.', default: process.cwd() },
        { name: '--platform', desc: 'The platform.', required: true, validator: ["firebase"] },
        { name: '--firebase-project', desc: 'The Firebase project name.' },
    ];

    async run(args: Args, opts: Opts, logger: Logger) {
        logger.info("==> Building...")
        // await board.build(opts.appDir);

        logger.info(`==> Deploying to ${opts.platform}`);
        const mod = require(path.resolve(__dirname, `../platform/${opts.platform}`));
        await mod.deploy(opts.appDir, {
            firebaseProject: opts.firebaseProject
        });

        logger.info(`==> Successfully deployed to ${opts.platform}`);
    }
}
