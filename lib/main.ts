import * as caporal from "caporal";
import * as fs from "fs";
import * as path from "path";
import { commands } from "./commands";
import { logger } from "./logger";
const { version } = require(path.resolve(__dirname, "../package.json"));

caporal.version(version);

// Load commands.
for (const klass of commands) {
    const cmd = caporal.command(klass.command, klass.desc);

    for (const arg of klass.args) {
        cmd.argument(arg.name, arg.desc, arg.validator, arg.default);
    }

    for (const opt of klass.opts) {
        cmd.option(opt.name, opt.desc, opt.validator, opt.default, opt.required);
    }

    cmd.action(async (args, opts) => {
        const instance = new klass();
        try {
            await instance.run(args, opts);
        } catch (e) {
            logger.error(e.message);
            logger.debug(e.stack);
            process.exit(1);
        }
    });
}

export function run(args?: string[]) {
    const argv = args ? [process.argv0, "makestack", ...args] : process.argv;
    return caporal.parse(argv);
}
