import { spawnSync } from "child_process";
import * as fs from "fs";
import * as path from "path";
import { api } from "../api";
import { createFile, run } from "../helpers";
import { logger } from "../logger";
import { prepare } from "../prepare";
import { FatalError } from "../types";
const nunjuck = require("nunjucks");

const DEFAULT_TEMPLATES = [
    {
        filepath: ".gitignore",
        template: `\
node_modules
`
    },
    {
        filepath: "README.md",
        template: `\
{{ appName }}
=============
`
    },
    {
        filepath: "app.yaml",
        template: `\
name: {{ appName }}
plugins: [ {{ plugins | join(", ") }} ]
`
    },
    {
        filepath: "jsconfig.json",
        template: `\
{
    "compilerOptions": {
        "target": "es2017",
        "module": "commonjs",
        "lib": ["es2017"]
    }
}
`
    },
    {
        filepath: "app.js",
        template: `\
const { Timer, println } = require('makestack/@runtime')

Timer.interval(3, () => {
    println('Hello World!')
})
`
    },
];

export async function main(args: any, opts: any) {
    const appDir = args.dir
    const appName = path.basename(appDir);
    const plugins = opts.plugins.split(",");
    const templates = DEFAULT_TEMPLATES;
    const context = {
        appName, plugins, appDir
    };

    if (fs.existsSync(appDir)) {
        throw new FatalError(`The file or directory with the same name already exists: \`${appDir}'`)
    }

    for (const { filepath, template } of templates) {
        const dest = path.join(appDir, filepath);
        logger.progress(`Creating ${dest}`);
        createFile(dest, nunjuck.renderString(template, context));
    }

    await prepare(appDir);

    let appJS = fs.readFileSync(path.join(appDir, "app.js"), {
        encoding: "utf-8",
    });

    for (const plugin of plugins) {
        const scaffoldPath = path.resolve(appDir, `node_modules/@makestack/${plugin}/scaffold.js`);
        if (fs.existsSync(scaffoldPath)) {
            logger.progress(`Scaffolding ${plugin}`);
            const { generateCodeHeader, generateCodeFooter, generateFiles } = require(scaffoldPath);
            if (generateCodeHeader) {
                appJS = generateCodeHeader(context).replace(/\n+$/, "") + "\n\n" + appJS
            }

            if (generateCodeFooter) {
                appJS = appJS.replace(/\n+$/, "") + "\n\n" + generateCodeFooter(context).replace(/\n+$/, "") + "\n"
            }

            if (generateFiles) {
                generateFiles(context)
            }
        }
    }

    if (appJS) {
        fs.writeFileSync(path.join(appDir, "app.js"), appJS);
    }

    logger.progress('Initializing a Git repository');
    run(['git', 'init'], { cwd: appDir });

    if (opts.register) {
        logger.progress('Registering the app');
        api.createApp(appName, opts.api);
    }
}
