const chalk = require("chalk");
import * as ejs from "ejs";
import * as fs from "fs-extra";
import * as path from "path";
import { logger } from "./logger";

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
        <p>This file is located at: <%= dir %>/public/index.html</p>
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

export interface ScaffoldArgs {
    name: string;
    dir: string;
    frontend?: string;
}"";

export function scaffold(args: ScaffoldArgs) {
    if (!fs.existsSync(args.dir)) {
        mkdir(args.dir);
    }

    genFile(path.join(args.dir, "package.json"), PACKAGE_JSON_TMPL, args);
    genFile(path.join(args.dir, "server.js"), SERVER_JS_TMPL, args);
    genFile(path.join(args.dir, "device.cc"), DEVICE_CC_TMPL, args);
    genFile(path.join(args.dir, ".gitignore"), GITIGNORE_TMPL, args);

    switch (args.frontend) {
        case "static":
            mkdir(path.join(args.dir, "public"));
            genFile(path.join(args.dir, "public/index.html"), INDEX_HTML_TMPL, args);
            break;
    }
}
