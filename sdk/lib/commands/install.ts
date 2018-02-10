const path = require("path");
const chalk = require("chalk");
const { install } = require("../install");
import { FatalError } from "../types";

function progress(stage: string, state: { type: string, percentage: number }) {
    switch (stage) {
        case "look-for-drive":
        console.info(chalk.bold.blue("==> (1/5) Looking for the drive"));
        break;
        case "register":
        console.info(chalk.bold.blue("==> (2/5) Registering the device"));
        break;
        case "download":
        console.info(chalk.bold.blue("==> (3/5) Downloading the disk image"));
        break;
        case "config":
        console.info(chalk.bold.blue("==> (4/5) Writing config"));
        break;
        case "flash":
        console.info(chalk.bold.blue("==> (5/5) Flashing"));
        break;
        case "flashing":
        const messages: { [name: string]: string } = { write: "Writing", check: "Verifying" };
        const message = messages[state.type];
        [state.type];
        console.info(`${message}...(${state.percentage}%)`);
        break;
    }
}

export async function main(args: any, opts: any, logger: any) {
    if (opts.wifiSsid && (!opts.wifiPassword || !opts.wifiCountry)) {
        throw new FatalError('--wifi-password and/or --wifi-country are missing.');
    }

    await install({
        deviceName: opts.name,
        deviceType: opts.type,
        osType: opts.os,
        adapter: opts.adapter,
        drive: opts.drive,
        flashCommand: [process.argv0, path.resolve(__dirname, "../../bin/makestack")],
        wifiSSID: opts.wifiSsid,
        wifiPassword: opts.wifiPassword,
        wifiCountry: opts.wifiCountry,
        diskImagePath: opts.image
    }, progress);

    console.info(chalk.bold.green("Done!"));
}
