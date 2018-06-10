import { spawnSync } from "child_process";
import * as fs from "fs-extra";
import * as os from "os";
import * as path from "path";
import { createFirmwareImage } from "../firmware";
import { Board } from "../types";
const packageJson = require("../../../package.json");

export class Esp32Board extends Board {
    public async build(appDir: string): Promise<void> {
        const esp32Dir = path.resolve(__dirname, "../../../esp32");

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

        console.log(`==> make in ${esp32Dir} version ${appVersion}`);
        const procs = os.cpus().length;
        const cp = spawnSync("/usr/bin/make", [`-j${procs}`], {
            stdio: "inherit",
            cwd: esp32Dir,
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

        if (cp.status != 0) {
            throw new Error("Failed to build the app.");
        }

        const rawImage = fs.readFileSync(path.join(esp32Dir, "build/firmware.bin"));
        const image = createFirmwareImage(appVersion, rawImage);
        fs.writeFileSync(path.join(appDir, "firmware.bin"), image);
    }
}
