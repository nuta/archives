import * as fs from "fs-extra";
import * as path from "path";
import { spawnSync } from "child_process";
import { DeployOptions } from "../../types";

export async function deploy(appDir: string, opts: DeployOptions) {
    if (!opts.firebaseProject) {
        throw new Error("--firebase-project is not set.");
    }

    const projDir = path.join(appDir, "build/deploy");
    const pkgDir = path.join(appDir, "build/deploy/functions");

    fs.mkdirpSync(pkgDir);

    fs.writeJsonSync(path.join(projDir, "firebase.json"), {
    })

    const packageJsonPath = path.resolve(__dirname, "../../../../package.json");
    const { dependencies } = require(packageJsonPath);
    const blacklist = ["serialport"];
    for (const dep of blacklist) {
        delete dependencies[dep];
    }

    fs.writeJsonSync(path.join(pkgDir, "package.json"), {
        private: true,
        main: "./index.js",
        dependencies
    });

    fs.mkdirpSync(path.join(pkgDir, "makestack"));
    fs.copyFileSync(
        packageJsonPath,
        path.join(pkgDir, "makestack/package.json")
    );

    fs.copyFileSync(
        path.join(appDir, "server.js"),
        path.join(pkgDir, "server.js")
    );

    fs.copyFileSync(
        path.join(appDir, "firmware.bin"),
        path.join(pkgDir, "firmware.bin")
    );

    fs.copyFileSync(
        path.resolve(__dirname, "start.js"),
        path.join(pkgDir, "index.js")
    );

    spawnSync("yarn", [
        "run",
        "babel",
        "--presets=es2015,stage-3",
        path.resolve(__dirname, "../../../../dist"),
        "--out-dir",
        path.join(pkgDir, "makestack/dist")
    ])

    spawnSync("yarn", { cwd: pkgDir, stdio: 'inherit' });
    spawnSync(
        "firebase",
        ["deploy", "--project", opts.firebaseProject],
        {
            stdio: 'inherit',
            cwd: projDir
        }
    );
}
