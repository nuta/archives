const chalk = require("chalk");
import * as child_process from "child_process";
import * as fs from "fs";
import * as path from "path";
import { board } from "../boards";
import { Args, Command, Opts } from "../cli";

export default class DeployCommand extends Command {
    public static command = "deploy";
    public static desc = "";
    public static args = [];
    public static opts = [
        { name: "--app-dir", desc: "The app directory.", default: process.cwd() },
        { name: "--platform", desc: "The platform.", required: true, validator: ["firebase"] },
        { name: "--firebase-project", desc: "The Firebase project name." },
    ];

    public async run(args: Args, opts: Opts, logger: Logger) {
        logger.info("==> Building...");
        // await board.build(opts.appDir);

        logger.info(`==> Deploying to ${opts.platform}`);
        const mod = require(path.resolve(__dirname, `../platform/${opts.platform}`));
        await mod.deploy(opts.appDir, {
            firebaseProject: opts.firebaseProject,
        });

        logger.info(`==> Successfully deployed to ${opts.platform}`);
    }
}
