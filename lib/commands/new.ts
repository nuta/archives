const chalk = require("chalk");
import * as ejs from "ejs";
import * as fs from "fs-extra";
import * as path from "path";
import { Args, CommandBase, Opts } from "../cli";
import { logger } from "../logger";

const DEVICE_CC_TMPL = `\
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

const SERVER_JS_TMPL = `\
`;

const PACKAGE_JSON_TMPL = `\
{
    "private": true,
    "makestack": {
        "production": {
            "platform": "firebase",
            "firebaseProject": "YOUR_FIREBASE_PROJECT"
        },
        "plugins": ["http_adapter"],
        "devPlugins": ["serial_adapter", "http_adapter"]
    }
}
`;

const GITIGNORE_TMPL = `\
*.firmware
build
`;

const INDEX_HTML_TMPL = `\
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Welcome to MakeStack!</title>
</head>
<body>
    <div style="width: 800px; margin: 50px auto 0">
        <h2>Welcome to MakeStack!</h2>
        <p>This file is located at: <%= appDir %>/public/index.html</p>
    </div>
</body>
</html>
`;

function genFile(filepath: string, template: string, ctx: any) {
    logger.action("create", filepath);
    fs.writeFileSync(filepath, ejs.render(template, ctx));
}

function mkdir(filepath: string) {
    logger.action("mkdir", filepath);
    fs.mkdirSync(filepath);
}

export class Command extends CommandBase {
    public static command = "new";
    public static desc = "Create a new app.";
    public static args = [
        { name: "dir", desc: "The directory." },
    ];
    public static opts = [
        {
            name: "--frontend",
            desc: "The frontend framework.",
            default: "static",
            validators: ["static"],
        },
    ];

    public async run(args: Args, opts: Opts) {
        const appDir = args.dir;
        const ctx = {
            appDir,
            name: path.basename(appDir),
        };

        mkdir(appDir);
        genFile(path.join(appDir, "package.json"), PACKAGE_JSON_TMPL, ctx);
        genFile(path.join(appDir, "server.js"), SERVER_JS_TMPL, ctx);
        genFile(path.join(appDir, "device.cc"), DEVICE_CC_TMPL, ctx);
        genFile(path.join(appDir, ".gitignore"), GITIGNORE_TMPL, ctx);

        switch (opts.frontend) {
            case "static":
                mkdir(path.join(appDir, "public"));
                genFile(path.join(appDir, "public/index.html"), INDEX_HTML_TMPL, ctx);
                break;
        }
    }
}
