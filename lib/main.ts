import * as caporal from "caporal";
import * as fs from "fs";
import * as path from "path";
const { version } = require("../../package.json");

caporal.version(version);

// Load commands.
const COMMANDS = [
    "new",
    "dev",
    "deploy",
];

for (const name of COMMANDS) {
    const modulePath = path.resolve(__dirname, "commands", name);
    const klass: any = require(modulePath).default;
    const cmd = caporal.command(klass.command, klass.desc);

    for (const arg of klass.args) {
        cmd.argument(arg.name, arg.desc, arg.validator, arg.default);
    }

    for (const opt of klass.opts) {
        cmd.option(opt.name, opt.desc, opt.validator, opt.default, opt.required);
    }

    cmd.action(async (args, opts, logger) => {
        const instance = new klass();
        await instance.run(args, opts, logger);
    });
}

export function run(args?: string[]) {
    const argv = args ? [process.argv0, "makestack", ...args] : process.argv;
    caporal.parse(argv);
}
