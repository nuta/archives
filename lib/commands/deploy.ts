const chalk = require("chalk");
import * as child_process from "child_process";
import * as fs from "fs";
import * as path from "path";
import { board } from "../boards";
import { logger } from "../logger";
import { Args, CommandBase, Opts } from "../cli";

export class Command extends CommandBase {
    public static command = "deploy";
    public static desc = "";
    public static args = [];
    public static opts = [
        { name: "--app-dir", desc: "The app directory.", default: process.cwd() },
        { name: "--platform", desc: "The platform.", required: true, validator: ["firebase"] },
        { name: "--firebase-project", desc: "The Firebase project name." },
    ];

    public async run(args: Args, opts: Opts) {
        logger.progress("Building the firmware");
        await board.build(opts.appDir);

        logger.progress(`Deploying to ${opts.platform}`);
        const mod = require(path.resolve(__dirname, `../platform/${opts.platform}`));
        await mod.deploy(opts.appDir, {
            firebaseProject: opts.firebaseProject,
        });

        logger.progress(`Successfully deployed to ${opts.platform}`);
    }
}
