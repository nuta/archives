import { spawnSync } from "child_process";
import * as fs from "fs";
import * as path from "path";
import { api } from "../api";
import { createFile, run } from "../helpers";
import { logger } from "../logger";
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
        filepath: "package.json",
        template: `\
{
  "name": "{{ appName }}",
  "private": true,
  "dependencies": {},
  "devDependencies": {}
}
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
const { Timer, println } = require('makestack')

Timer.interval(3, () => {
  println('Hello World!')
})
`
    },
];

export async function main(args: any, opts: any) {
    const appDir = args.dir
    const appName = path.basename(appDir);
    const templates = DEFAULT_TEMPLATES;
    const context = { appName, appDir };

    if (fs.existsSync(appDir)) {
        throw new FatalError(`The file or directory with the same name already exists: \`${appDir}'`)
    }

    for (const { filepath, template } of templates) {
        const dest = path.join(appDir, filepath);
        logger.progress(`Creating ${dest}`);
        createFile(dest, nunjuck.renderString(template, context));
    }

    logger.progress('Initializing a Git repository');
    run(['git', 'init'], { cwd: appDir });

    if (opts.register) {
        logger.progress('Registering the app');
        api.createApp(appName, opts.api);
    }
}
