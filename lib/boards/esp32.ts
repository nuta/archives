import { spawnSync, SpawnSyncReturns } from "child_process";
import * as fs from "fs-extra";
import * as os from "os";
import * as path from "path";
import { logger } from "../logger";
import { createFirmwareImage } from "../firmware";
import { Board } from "../types";
const packageJson = require("../../../package.json");

function make(esp32Dir: string, firmwareVersion: string, appVersion: number): any {
    const procs = os.cpus().length;
    return spawnSync("/usr/bin/make", [`-j${procs}`], {
        cwd: esp32Dir,
        encoding: 'utf-8',
        env: {
            PATH: process.env.PATH,
            APP_OBJS: "app.o",
            WIFI_SSID: process.env.WIFI_SSID,
            WIFI_PASSWORD: process.env.WIFI_PASSWORD,
            SERVER_URL: process.env.SERVER_URL,
            CA_CERT: process.env.CA_CERT,
            DEVICE_ID: process.env.DEVICE_ID,
            RELEASE: "", // debug build
            APP_VERSION: appVersion,
            FIRMWARE_VERSION: packageJson.version,
        },
    });
}

export class Esp32Board extends Board {
    public async build(appDir: string): Promise<void> {
        const esp32Dir = path.resolve(__dirname, "../../../esp32");

        if (!fs.existsSync(path.join(esp32Dir, "deps"))) {
            spawnSync("./tools/download-dependencies", {
                stdio: 'inherit',
                cwd: esp32Dir
            });
        }

        fs.mkdirpSync(path.join(esp32Dir, "build"));
        fs.copyFileSync(
            path.join(appDir, "device.cc"),
            path.join(esp32Dir, "src/app.cc"),
        );

        // Update src/component.mk to rebuild source files depends
        // on WIFI_SSID, etc.
        const mk = path.join(esp32Dir, "src/component.mk");
        fs.writeFileSync(mk, fs.readFileSync(mk));

        // Use an (almost) unique number as the app version.
        const appVersion = process.hrtime()[0] % 4200000000;

        const cp = make(esp32Dir, packageJson.version, appVersion);
        if (cp.status != 0) {
            if (cp.stderr.includes("No rule to make target `cores/esp32/libb64/cencode.o'")) {
                // Try again because the build system is broken. TODO: fixme
                const cp2 = make(esp32Dir, packageJson.version, appVersion);
                if (cp2.status != 0) {
                        console.log(cp2.stdout);
                        console.log(cp2.stderr);
                        throw new Error("Failed to build the app.");
                }
            } else {
                console.log(cp.stdout);
                console.log(cp.stderr);
                throw new Error("Failed to build the app.");
            }
        }


        const rawImage = fs.readFileSync(path.join(esp32Dir, "build/firmware.bin"));
        const image = createFirmwareImage(appVersion, rawImage);
        fs.writeFileSync(path.join(appDir, "firmware.bin"), image);
    }
}
