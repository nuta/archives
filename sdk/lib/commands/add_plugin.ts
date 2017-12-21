import { spawnSync } from "child_process";
import * as fs from "fs";
import * as path from "path";
import { api } from "../api";
import { createFile, run } from "../helpers";
import { logger } from "../logger";
import { FatalError } from "../types";
import { loadAppYAML } from "../appdir";

export async function main(args: any, opts: any) {
    const appName = loadAppYAML(opts.appDir).name;

    logger.progress(`Adding @makestack/${args.plugin} by yarn`)
    run(['yarn', 'add', `@makestack/${args.plugin}`])


    const scaffoldPath = path.resolve(
        opts.appDir, `node_modules/@makestack/${args.plugin}/scaffold.js`
    )

    if (fs.existsSync(scaffoldPath)) {
        logger.progress(`Scaffolding ${args.plugin}`)

        const { scaffold } = require(scaffoldPath)
        const context = {}
        const { header = '', footer = '' } = scaffold(context)

        let appJS = fs.readFileSync(path.join(opts.appDir, 'app.js'), { encoding: 'utf-8' })
        appJS =
            header.replace(/\n+$/, '') + "\n\n" +
            appJS.replace(/\n+$/, '') + "\n\n" +
            footer.replace(/\n+$/, '') + "\n"

        fs.writeFileSync(path.join(args.dir, 'app.js'), appJS)
    }
}
