const chalk = require("chalk");
import * as child_process from "child_process";
import * as fs from "fs-extra";
import * as path from "path";
import { board } from "../boards";
import { Args, CommandBase, constructEnvOption, Opts } from "../cli";
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
        const version = (config.production.version || 0) + 1;
        await board.build(true, version, repoDir, opts.appDir);

        logger.progress(`Deploying to ${config.platform}`);
        await getSdkInstance(config.platform).deploy(opts.appDir, config);
        logger.progress(`Successfully deployed #${version} to ${config.platform}`);

        const packageJsonPath = path.join(opts.appDir, "package.json");
        const packageJson = fs.readJsonSync(packageJsonPath);
        packageJson.makestack.production.version = version;
        fs.writeFileSync(packageJsonPath, JSON.stringify(packageJson, null, 2));
    }
}
