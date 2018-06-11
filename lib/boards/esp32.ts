import { spawnSync, SpawnSyncReturns } from "child_process";
import * as fs from "fs-extra";
import * as os from "os";
import * as path from "path";
import { logger } from "../logger";
import { createFirmwareImage } from "../firmware";
import { Board, InstallConfig } from "../types";
const packageJson = require("../../../package.json");

function make(esp32Dir: string, firmwareVersion: string, appVersion: number): any {
    const procs = os.cpus().length;
    // TODO: release build
    return spawnSync("/usr/bin/make", [`-j${procs}`], {
        cwd: esp32Dir,
        encoding: 'utf-8',
        env: {
            PATH: process.env.PATH,
            APP_OBJS: "app.o",
            RELEASE: "", // debug build
            APP_VERSION: appVersion,
            FIRMWARE_VERSION: packageJson.version,
        },
    });
}

export class Esp32Board extends Board {
    private prepareEsp32Dir(): string {
        return path.resolve(__dirname, "../../../esp32");
    }

    public async build(appDir: string): Promise<void> {
        const esp32Dir = this.prepareEsp32Dir();

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

        fs.copyFileSync(
            path.join(esp32Dir, "build/firmware.bin"),
            path.join(appDir, "firmware.bin"),
        );
    }

    public async install(appDir: string, config: InstallConfig): Promise<void> {
        if (!config.serial) {
            throw new Error("serial is not set")
        }

        const rawImage = fs.readFileSync(path.join(appDir, "firmware.bin"));
        const firmwarePath =  path.join(appDir, `firmware.${config.deviceName}.bin`);
        fs.writeFileSync(firmwarePath, createFirmwareImage(rawImage, config));

        const esp32Dir = this.prepareEsp32Dir();
        const args = [
            path.resolve(esp32Dir, "deps/esp-idf/components/esptool_py/esptool/esptool.py"),
            "--port", config.serial,
            "--baud", "921600",
            "--chip", "esp32",
            "--before", "default_reset", "--after", "hard_reset" ,
            "write_flash",
            "-z", "--flash_mode", "dio", "--flash_freq", "80m", "--flash_size", "detect",
            "0xe000", path.resolve(esp32Dir, "deps/arduino-esp32/tools/partitions/boot_app0.bin"),
            "0x1000", "bootloader/bootloader.bin",
            "0x10000", "firmware.bin",
            "0x8000", "default.bin"
        ]

        // TODO: ensure that pyserial is installed
        const { status } = spawnSync("python", args, {
            stdio: 'inherit',
            cwd: path.join(esp32Dir, "build")
        })

        if (status != 0) {
            throw new Error("Failed to install the firmware.");
        }
    }
}
