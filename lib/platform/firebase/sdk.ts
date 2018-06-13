import { spawnSync } from "child_process";
import * as fs from "fs-extra";
import * as path from "path";
import * as EventEmitter from "events";
import * as readline from "readline";
import { PlatformSdk } from "../sdk";
import { DeployOptions } from "../../types";
import { execWithPipe } from "../../helpers";
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
        const { dependencies } = require(packageJsonPath);
        const blacklist = ["serialport"];
        for (const dep of blacklist) {
            delete dependencies[dep];
        }

        fs.writeJsonSync(path.join(pkgDir, "package.json"), {
            private: true,
            main: "./index.js",
            dependencies,
        });

        fs.mkdirpSync(path.join(pkgDir, "makestack"));
        fs.copyFileSync(
            packageJsonPath,
            path.join(pkgDir, "makestack/package.json"),
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

        spawnSync("yarn", [
            "run",
            "babel",
            "--presets=es2015,stage-3",
            path.resolve(__dirname, "../../../../dist"),
            "--out-dir",
            path.join(pkgDir, "makestack/dist"),
        ]);

        spawnSync("yarn", { cwd: pkgDir, stdio: "inherit" });
        spawnSync(
            "firebase",
            ["deploy", "--project", opts.firebaseProject],
            {
                stdio: "inherit",
                cwd: projDir,
            },
        );
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
