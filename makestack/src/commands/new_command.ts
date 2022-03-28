import * as fs from "fs";
import * as path from "path";
import * as nunjucks from "nunjucks";
import { Args, Command, Opts } from "./command";
import { logger } from "../logger";
import { exec, UserError } from "../helpers";

const PACKAGE_JSON = `\
{
    "private": true,
    "makestack": {
        "name": "{{ name }}",
        "board": "esp32",
        "cloud": "firebase"
    },
    "scripts": {
{% if typescript %}
        "build": "./node_modules/.bin/tsc app.ts"
{% endif %}
    }
}
`

const APP_JS = `\
const app = require("makestack")

app.onReady((device) => {
    const LED_PIN = 22
    device.pinMode(LED_PIN, "OUTPUT")
    while (1) {
        device.print("Blinking!")
        device.digitalWrite(LED_PIN, true)
        device.delay(1000)
        device.digitalWrite(LED_PIN, false)
        device.delay(1000)
    }
})
`


const APP_TS = `\
import * as app from "makestack";
import { DeviceAPI } from "makestack";

app.onReady((device: DeviceAPI) => {
    const LED_PIN = 22;
    device.pinMode(LED_PIN, "OUTPUT");
    while (1) {
        device.print("Blinking!");
        device.digitalWrite(LED_PIN, true);
        device.delay(1000);
        device.digitalWrite(LED_PIN, false);
        device.delay(1000);
    }
});
`

const TSCONFIG_JSON = `\
{
    "compilerOptions": {
        "declaration": true,
        "sourceMap": true,
        "module": "commonjs",
        "moduleResolution": "node",
        "pretty": true,
        "alwaysStrict": true,
        "strict": true,
        "target": "es2018"
    },
    "include": [
        "./src/**/*"
    ]
}
`

const FIREBASE_JSON = `\
{
    "functions": {
    },
    "hosting": {
        "public": "public",
        "ignore": [
            "firebase.json",
            "**/.*",
            "**/node_modules/**"
        ]
    }
}
`

const INDEX_HTML = `\
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>{{ name }} - index.html</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Oxygen, Ubuntu, Cantarell, 'Open Sans', 'Helvetica Neue', sans-serif;
            color: #121212;
        }
    </style>
</head>
<body>
    <div style="width: 500px; margin: 70px auto 0;">
        <h1 style="text-align: center;">{{name }} - index.html</h1>
        <hr>
        <p>
            The <code>makestack deploy</code> command will automatically uploads
            files in this <code>public</code> directory to the platform, for instance,
            <a href="https://firebase.google.com/docs/hosting">Firebase Hosting</a>.
        </p>
    </div>
</body>
</html>
`

const GITIGNORE = `\
node_modules
*.log
app.js
`

const DEPENDENCIES: string[] = ["makestack"]
const DEV_DEPENDENCIES: string[] = []
const TYPESCRIPT_DEV_DEPENDENCIES: string[] = ["typescript"]

function genFile(filepath: string, template: string, ctx: any) {
    logger.action("create", filepath);
    nunjucks.configure({ autoescape: false });
    fs.writeFileSync(filepath, nunjucks.renderString(template, ctx));
}

function mkdir(filepath: string) {
    logger.action("mkdir", filepath);
    fs.mkdirSync(filepath);
}

interface ScaffoldOptions {
    typescript: boolean,
    firebase: boolean,
}

function scaffold(appDir: string, opts: ScaffoldOptions) {
    if (fs.existsSync(appDir)) {
        throw new UserError(`The directory alredy exists: \`${appDir}'`);
    }

    logger.progress("Generating files");
    mkdir(appDir);
    const ctx = {
        name: path.basename(appDir),
        typescript: opts.typescript,
    };

    genFile(path.join(appDir, "package.json"), PACKAGE_JSON, ctx);
    genFile(path.join(appDir, ".gitignore"), GITIGNORE, ctx);
    if (opts.typescript) {
        genFile(path.join(appDir, "app.ts"), APP_TS, ctx);
        genFile(path.join(appDir, "tsconfig.json"), TSCONFIG_JSON, ctx);
    } else {
        genFile(path.join(appDir, "app.js"), APP_JS, ctx);
    }

    if (opts.firebase) {
        genFile(path.join(appDir, "firebase.json"), FIREBASE_JSON, ctx);
        mkdir(path.join(appDir, "public"));
        genFile(path.join(appDir, "public/index.html"), INDEX_HTML, ctx);
    }

    logger.progress("Installing dependencies...");
    let deps = [...DEPENDENCIES, ...DEV_DEPENDENCIES];
    if (opts.typescript) {
        deps = deps.concat(TYPESCRIPT_DEV_DEPENDENCIES);
    }
    exec(["yarn", "add", ...deps], { cwd: appDir });

    logger.success(`Successfully generated ${appDir}`);
}

export class NewCommand extends Command {
    public static command = "new";
    public static desc = "Create a new app.";
    public static args = [
        {
            name: "path",
            desc: "The app directory.",
        }
    ];
    public static opts = [
        {
            name: "--typescript",
            desc: "Create a TypeScript app.",
            default: false,
        },
        {
            name: "--firebase",
            desc: "Generate Firebase files.",
            default: false,
        },
    ];

    public async run(args: Args, opts: Opts) {
        await scaffold(args.path, opts as ScaffoldOptions);
    }
}
