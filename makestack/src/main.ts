import * as caporal from "caporal";
import * as fs from "fs";
import * as path from "path";
import { commands } from "./commands";
import { logger } from "./logger";
import { WATCH_OPTS, OptDefinition } from "./commands/command";
import { UserError } from "./helpers";
const { version } = require(path.resolve(__dirname, "../package.json"));

caporal.description("A minimalistic IoT framework for super-rapid prototyping.");
caporal.version(version);

// Load commands.
for (const klass of commands) {
    const cmd = caporal.command(klass.command, klass.desc);

    for (const arg of klass.args) {
        cmd.argument(arg.name, arg.desc, arg.validator, arg.default);
    }

    let opts: OptDefinition[] = klass.opts;
    if (klass.watchMode) {
        opts = [...opts, ...WATCH_OPTS];
    }

    let addedOpts: string[] = [];
    for (const opt of opts) {
        if (!addedOpts.includes(opt.name)) {
            cmd.option(opt.name, opt.desc, opt.validator, opt.default, opt.required);
        }

        addedOpts.push(opt.name);
    }

    cmd.action(async (args, opts) => {
        const instance = new klass();
        try {
            await instance.run(args, opts);
            if (opts.watch) {
                logger.info("Watching app.js for changes...")
                fs.watchFile(path.join(opts.appDir, "app.js"), () => {
                    instance.run(args, opts);
                });
            }
        } catch (e) {
            if (e instanceof UserError) {
                logger.error(e.message);
            } else {
                logger.error(e.stack);
            }
            process.exit(1);
        }
    });
}

export function run(args?: string[]) {
    const argv = args ? [process.argv0, "makestack", ...args] : process.argv;
    return caporal.parse(argv);
}
