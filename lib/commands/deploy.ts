const chalk = require("chalk");
import * as child_process from "child_process";
import * as fs from "fs";
import * as path from "path";
import { board } from "../boards";
import { Args, CommandBase, Opts, constructEnvOption } from "../cli";
import { downloadRepo, loadConfig } from "../helpers";
import { logger } from "../logger";
import { getSdkInstance } from "../platform";

export class Command extends CommandBase {
    public static command = "deploy";
    public static desc = "";
    public static args = [];
    public static opts = [
        { name: "--app-dir", desc: "The app directory.", default: process.cwd() },
        { name: "--repo-dir", desc: "The file path to the seiyanuta/makestack repo." },
        constructEnvOption("production"),
    ];

    public async run(args: Args, opts: Opts) {
        const config = loadConfig(opts.appDir, opts.env);
        const repoDir = opts.repoDir || downloadRepo(opts.appDir);

        logger.progress("Building the firmware");
        await board.build(repoDir, opts.appDir);

        logger.progress(`Deploying to ${config.platform}`);
        await getSdkInstance(config.platform).deploy(opts.appDir, config);
        logger.progress(`Successfully deployed to ${config.platform}`);
    }
}
