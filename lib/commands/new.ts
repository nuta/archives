const chalk = require("chalk");
import * as ejs from "ejs";
import * as fs from "fs-extra";
import * as path from "path";
import { Args, Command, Opts } from "../cli";

const DEVICE_CC_TMPL = `
#include "makestack.h"

void loop() {
    delay(100);
    digitalWrite(12, HIGH);
    delay(100);
    digitalWrite(12, LOW);
}

void setup() {
    pinMode(12, OUTPUT);
}
`;

const SERVER_JS_TMPL = `
`;

const GITIGNORE_TMPL = `
firmware.bin
build
`;

function progressMsg(action: string, target: string): string {
    return `${chalk.blue.bold(action.padStart(8))}  ${target}`;
}

function genFile(filepath: string, template: string, ctx: any) {
    fs.writeFileSync(filepath, ejs.render(template, ctx));
}

export default class NewCommand extends Command {
    public static command = "new";
    public static desc = "Create a new app.";
    public static args = [
        { name: "dir", desc: "The directory." },
    ];
    public static opts = [];

    public async run(args: Args, opts: Opts, logger: Logger) {
        const ctx = {
            name: path.basename(args.dir),
        };

        logger.info(progressMsg("mkdir", args.dir));
        fs.mkdirSync(args.dir);
        logger.info(progressMsg("create", path.join(args.dir, "app.js")));
        genFile(path.join(args.dir, "server.js"), SERVER_JS_TMPL, ctx);
        genFile(path.join(args.dir, "device.cc"), DEVICE_CC_TMPL, ctx);
        genFile(path.join(args.dir, ".gitignore"), GITIGNORE_TMPL, ctx);
    }
}
