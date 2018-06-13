import * as fs from "fs-extra";
import * as path from "path";
import * as EventEmitter from "events";
import * as readline from "readline";
import { PlatformSdk } from "../sdk";
import { DeployOptions } from "../../types";
import { execWithPipe, exec } from "../../helpers";
import { logger } from "../../logger";
const { blue } = require("chalk");

export class FirebasePlatformSdk extends PlatformSdk {
    async command(deviceName: string, command: string, arg: string): Promise<any> {
        // TODO
        return {};
    }

    async deploy(appDir: string, opts: DeployOptions) {
        if (!opts.firebaseProject) {
            throw new Error("--firebase-project is not set.");
        }

        const projDir = path.join(appDir, "build/deploy");
        const pkgDir = path.join(appDir, "build/deploy/functions");

        fs.mkdirpSync(pkgDir);

        fs.writeJsonSync(path.join(projDir, "firebase.json"), {
            hosting: {
                public: "public",
                ignore: [
                    "**/.*",
                ],
                rewrites: [
                    {
                        source: "/makestack/**",
                        function: "makestack",
                    },
                ],
            },
        });

        const packageJsonPath = path.resolve(__dirname, "../../../../package.json");
        const pluginsDir = path.resolve(__dirname, "../../../../plugins");
        const { dependencies } = fs.readJsonSync(packageJsonPath);
        const blacklist = ["serialport"];
        for (const dep of blacklist) {
            delete dependencies[dep];
        }

        const appPackageJson = fs.readJsonSync(path.resolve(appDir, "package.json"));
        fs.writeJsonSync(path.join(pkgDir, "package.json"), {
            private: true,
            main: "./index.js",
            dependencies: Object.assign({}, dependencies, appPackageJson.dependencies),
            makestack: appPackageJson.makestack,
        });

        fs.mkdirpSync(path.join(pkgDir, "makestack"));
        fs.copyFileSync(
            packageJsonPath,
            path.join(pkgDir, "makestack/package.json"),
        );

        fs.copySync(
            path.join(pluginsDir),
            path.join(pkgDir, "makestack/plugins"),
        );

        fs.copySync(
            path.join(appDir, "public"),
            path.join(projDir, "public"),
        );

        fs.copyFileSync(
            path.join(appDir, "server.js"),
            path.join(pkgDir, "server.js"),
        );

        fs.copyFileSync(
            path.join(appDir, "firmware.esp32.bin"),
            path.join(pkgDir, "firmware.esp32.bin"),
        );

        fs.copyFileSync(
            path.resolve(__dirname, "start.js"),
            path.join(pkgDir, "index.js"),
        );

        logger.progress("Installing dependencies");
        exec(["yarn"], { cwd: pkgDir });

        logger.progress("Transpiling");
        exec(["yarn", "run", "babel", "--presets=es2015,stage-3",
            "--out-dir", path.join(pkgDir, "makestack/dist"),
            path.resolve(__dirname, "../../../../dist"),
        ], { cwd: pkgDir });

        logger.progress("firebase deploy (it may takes long)");
        exec(["firebase", "deploy", "--project", opts.firebaseProject], { cwd: projDir });
    }

    public viewLog(opts: any) {
        let lastLine: string | undefined;
        const interval = 5;
        let current = interval;

        const readLog = () => {
            const argv = ["firebase", "functions:log", "--project", opts.firebaseProject];
            const { stdout } = execWithPipe(argv);
            lastLine = stdout.split("\n").filter((l) => l.length > 0).pop();
            return stdout;
        };

        const printCountdown = () => {
            process.stdout.write(blue.bold(`==> Reloading in ${current} seconds...`));
        };

        const loop = () => {
            readline.clearLine(process.stdout, 0);
            readline.cursorTo(process.stdout, 0);
            current--;
            if (current > 0) {
                printCountdown();
                return;
            }

            let stdout = readLog();

            if (lastLine && stdout.includes(lastLine)) {
                stdout = stdout.split(lastLine, 2)[1];
            }

            if (stdout !== "\n") {
                process.stdout.write(stdout);
            }

            current = interval;
            printCountdown();
            resetTimer();
        };

        let timer: NodeJS.Timer | undefined;
        const resetTimer = () => {
            if (timer) {
                clearInterval(timer);
            }

            timer = setInterval(loop, 1000);
        };

        process.stdout.write(readLog());
        printCountdown();
        resetTimer();
    }
}
