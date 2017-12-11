import { spawnSync } from "child_process";
import * as fs from "fs";
import * as path from "path";
import { createFile, run } from "../helpers";
import { logger } from "../logger";
import { prepare } from "../prepare";
const nunjuck = require("nunjucks");

const DEFAULT_TEMPLATES = [
    {
        filepath: "README.md",
        template: `\
{{ appName }}
=============
`,
    },
    {
        filepath: "plugin.yaml",
        template: `\
name: {{ appName }}
`,
    },
    {
        filepath: ".gitignore",
        template: `\
/dist
/build
/native
`,
    },
];

const JAVASCRIPT_TEMPLATES = [
    {
        filepath: "package.json",
        template: `\
{
    "private": true,
    "main": "lib/index.js"
}
`,
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
`,
    },
    {
        filepath: "lib/index.js",
        template: `\
class {{ CamelAppName }} {
    constructor() {
        print('Hello form {{ CamelAppName }}!')
    }
}

 module.exports = { {{ CamelAppName }} }
`,
    },
    {
        filepath: ".makestackignore",
        template: `\
dist/*.map
scaffold.js
coverage
test
lib/native
node_modules/nan
node_modules/.yarn-integrity
binding.gyp
build
node_modules
package.json
jsconfig.json
yarn.lock
package-lock.json
`,
    },
];

const TYPESCRIPT_TEMPLATES = [
    {
        filepath: "package.json",
        template: `\
{
    "private": true,
    "main": "dist/index.js",
    "scripts": {
        "transpile": "tsc --pretty"
    }
}
`,
    },
    {
        filepath: "tsconfig.json",
        template: `\
{
    "compilerOptions": {
        "outDir": "./dist",
        "allowJs": true,
        "target": "es2017",
        "module": "commonjs",
        "sourceMap": true,
        "types": ["node", "makestack"]
    },
    "include": [
        "./lib/**/*"
    ]
}
`,
    },
    {
        filepath: "lib/index.ts",
        template: `\
const { println } = require('makestack/@runtime)

export class {{ CamelAppName }} {
    constructor() {
        println('Hello form {{ CamelAppName }}!')
    }
}
`,
    },
    {
        filepath: ".gitignore",
        template: `\
node_modules
build
native
coverage
`
    },
    {
        filepath: ".makestackignore",
        template: `\
dist/*.map
scaffold.js
coverage
test
lib
node_modules/nan
node_modules/.yarn-integrity
binding.gyp
build
node_modules
package.json
tsconfig.json
yarn.lock
package-lock.json
`,
    },
];

function toCamelCase(str: string) {
    str = str.replace(/(\-[a-z])/g, (m) => m[1].toUpperCase());
    return str.charAt(0).toUpperCase() + str.slice(1);
}

export async function main(args: any, opts: any) {
    const appName = path.basename(args.dir);
    const CamelAppName = toCamelCase(appName);

    let templates = DEFAULT_TEMPLATES;
    switch (opts.lang) {
        case "javascript":
            templates = templates.concat(JAVASCRIPT_TEMPLATES);
            break;
        case "typescript":
            templates = templates.concat(TYPESCRIPT_TEMPLATES);
            break;
    }

    const context = {
        appName, CamelAppName,
    };

    for (const { filepath, template } of templates) {
        logger.progress(`Creating ${filepath}`);
        createFile(path.join(args.dir, filepath), nunjuck.renderString(template, context));
    }

    if (opts.lang === "typescript") {
        logger.progress("Installing TypeScript dependencies");
        const deps = ["typescript", "@types/node"];
        spawnSync("yarn", ["add", "--dev"].concat(deps), {
            encoding: "utf-8",
            cwd: args.dir,
        });
    }

    logger.progress('Initializing a Git repository');
    run(['git', 'init'], { cwd: args.dir });

    await prepare(args.dir);
}
