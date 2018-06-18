import * as EventEmitter from "events";
import * as fs from "fs-extra";
import * as path from "path";
import * as readline from "readline";
import { execCmd, execCmdWithPipe } from "../../helpers";
import { logger } from "../../logger";
import { DeployOptions } from "../../types";
import { PlatformSdk } from "../sdk";
const { blue } = require("chalk");

const FIREBASE_JSON_TMPL = JSON.stringify({
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

export class FirebasePlatformSdk extends PlatformSdk {
    public async command(deviceName: string, command: string, arg: string): Promise<any> {
        // TODO
        return {};
    }

    private preparePackageJsonForDeploy(packageJsonPath: string, projDir: string, appDir: string) {
        fs.writeJsonSync(path.join(projDir, "firebase.json"), FIREBASE_JSON_TMPL);
        const { dependencies } = fs.readJsonSync(packageJsonPath);
        const blacklist = ["serialport"];
        for (const dep of blacklist) {
            delete dependencies[dep];
        }

        const appPackageJson = fs.readJsonSync(path.resolve(appDir, "package.json"));
        return {
            private: true,
            main: "./index.js",
            dependencies: Object.assign({}, dependencies, appPackageJson.dependencies),
            makestack: appPackageJson.makestack,
        };
    }

    public async deploy(appDir: string, opts: DeployOptions) {
        if (!opts.firebaseProject) {
            throw new Error("--firebase-project is not set.");
        }

        const topDir = path.resolve(__dirname, "../../..");
        const projDir = path.join(appDir, "build/deploy");
        const pkgDir = path.join(appDir, "build/deploy/functions");
        const pluginsDir = path.resolve(topDir, "lib/plugins");
        const packageJsonPath = path.resolve(topDir, "package.json");
        fs.mkdirpSync(pkgDir);
        const packageJson = this.preparePackageJsonForDeploy(packageJsonPath, projDir, appDir);
        fs.writeJsonSync(path.join(pkgDir, "package.json"), packageJson);
        fs.mkdirpSync(path.join(pkgDir, "makestack"));
        fs.copySync(packageJsonPath, path.join(pkgDir, "makestack/package.json"));
        fs.copySync(path.join(pluginsDir), path.join(pkgDir, "makestack/lib/plugins"));
        fs.copySync(path.join(appDir, "public"), path.join(projDir, "public"));
        fs.copySync(path.join(appDir, "server.js"), path.join(pkgDir, "server.js"));
        fs.copySync(path.join(appDir, "esp32.firmware"), path.join(pkgDir, "esp32.firmware"));
        fs.copySync(path.resolve(topDir, "dist/platform/firebase/start.js"), path.join(pkgDir, "index.js"));

        logger.progress("Installing dependencies");
        execCmd(["yarn"], { cwd: pkgDir });

        logger.progress("Transpiling");
        execCmd([
            "yarn", "run", "babel", "--presets=es2015,stage-3",
            "--out-dir", path.join(pkgDir, "makestack/dist"),
            path.resolve(topDir, "dist"),
        ], { cwd: pkgDir });

        logger.progress("firebase deploy (it may takes long)");
        execCmd(["firebase", "deploy", "--project", opts.firebaseProject], { cwd: projDir });
    }

    public viewLog(opts: any) {
        let lastLine: string | undefined;
        const interval = 5;
        let current = interval;

        const readLog = () => {
            const argv = ["firebase", "functions:log", "--project", opts.firebaseProject];
            const { stdout } = execCmdWithPipe(argv);
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
