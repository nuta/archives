const chalk = require("chalk");
import * as child_process from "child_process";
import * as fs from "fs";
import * as inquirer from "inquirer";
import * as path from "path";
import { board } from "../boards";
import { Args, CommandBase, constructEnvOption, Opts } from "../cli";
import { downloadRepo, generateRandomVersion } from "../helpers";
import { logger } from "../logger";
import { InstallConfig } from "../types";

const installConfigOptions = [
    { name: "--adapter", desc: "The network adapter." },
    { name: "--wifi-ssid", desc: "Wi-Fi SSID." },
    { name: "--wifi-password", desc: "Wi-Fi Password." },
    { name: "--server-url", desc: "The server URL." },
    { name: "--device-name", desc: "The device name." },
    { name: "--serial", desc: "The serial device file." },
];

async function getOrAsk(opts: { [name: string]: string }, questions: inquirer.Question[]) {
    for (const q of questions) {
        if (q.name && !opts[q.name]) {
            opts[q.name] = (await inquirer.prompt(q))[q.name];
        }
    }

    return opts;
}

async function getInstallConfigOrAsk(opts: { [name: string]: string }): Promise<InstallConfig> {
    const filled = await getOrAsk(opts, [
        { name: "adapter", message: "The network adapter", type: "list", choices: ["wifi", "sakuraio"] },
        { name: "wifiSsid", message: "Wi-Fi SSID", type: "input" },
        { name: "wifiPassword", message: "Wi-Fi Password", type: "password" },
        { name: "serverUrl", message: "The server URL", type: "input" },
        { name: "deviceName", message: "The device name", type: "input" },
        { name: "serial", message: "The serial device file path", type: "input" },
    ]);

    return (filled as any) as InstallConfig;
}

export class Command extends CommandBase {
    public static command = "install";
    public static desc = "";
    public static args = [];
    public static opts = [
        { name: "--app-dir", desc: "The app directory.", default: process.cwd() },
        { name: "--repo-dir", desc: "The file path to the seiyanuta/makestack repo." },
        ...installConfigOptions,
        constructEnvOption("development"),
    ];

    public async run(args: Args, opts: Opts) {
        const repoDir = opts.repoDir || downloadRepo(opts.appDir);
        const isReleaseBuild = (opts.env == "production") ? true : false;

        // We don't have the latest firmware image in production. Use 0 for now.
        const version = isReleaseBuild ? 0 : generateRandomVersion();

        logger.progress("Building the firmware");
        await board.build(isReleaseBuild, version, repoDir, opts.appDir);
        logger.progress("Installing the firmware");
        await board.install(repoDir, opts.appDir, await getInstallConfigOrAsk(opts));
        logger.progress(`Successfully installed to ${opts.serial}`);
    }
}
