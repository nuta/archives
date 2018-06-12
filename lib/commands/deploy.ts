const chalk = require("chalk");
import * as child_process from "child_process";
import * as fs from "fs";
import * as path from "path";
import { board } from "../boards";
import { logger } from "../logger";
import { Args, CommandBase, Opts } from "../cli";
import { downloadRepo } from "../helpers";
import { loadPlatform } from "../platform";

export class Command extends CommandBase {
    public static command = "deploy";
    public static desc = "";
    public static args = [];
    public static opts = [
        { name: "--app-dir", desc: "The app directory.", default: process.cwd() },
        { name: "--repo-dir", desc: "The file path to the seiyanuta/makestack repo." },
        { name: "--platform", desc: "The platform.", required: true, validator: ["firebase"] },
        { name: "--firebase-project", desc: "The Firebase project name." },
    ];

    public async run(args: Args, opts: Opts) {
        const repoDir = opts.repoDir || downloadRepo(opts.appDir);

        logger.progress("Building the firmware");
        await board.build(repoDir, opts.appDir);

        logger.progress(`Deploying to ${opts.platform}`);
        await loadPlatform(opts.platform).deploy(opts.appDir, {
            firebaseProject: opts.firebaseProject,
        });

        logger.progress(`Successfully deployed to ${opts.platform}`);
    }
}
