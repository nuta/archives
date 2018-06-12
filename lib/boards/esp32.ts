import { spawnSync, SpawnSyncReturns } from "child_process";
import * as fs from "fs-extra";
import * as os from "os";
import * as path from "path";
import { logger } from "../logger";
import { createFirmwareImage, removeFirmwareHeader, prepareFirmware } from "../firmware";
import { Board, InstallConfig } from "../types";
import { loadPlugins } from "../plugins";
const packageJson = require("../../../package.json");

function make(esp32Dir: string, firmwareVersion: string, appVersion: number): any {
    const procs = os.cpus().length;
    // TODO: release build
    return spawnSync("/usr/bin/make", [`-j${procs}`], {
        cwd: esp32Dir,
        encoding: 'utf-8',
        env: {
            PATH: process.env.PATH,
            MY_COMPONENTS: "app",
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
        const appComponentDir = path.join(esp32Dir, "app");
        const config = fs.readJsonSync(path.join(appDir, "package.json")).makestack;

        if (!fs.existsSync(path.join(esp32Dir, "deps"))) {
            spawnSync("./tools/download-dependencies", {
                stdio: 'inherit',
                cwd: esp32Dir
            });
        }

        fs.mkdirpSync(appComponentDir);
        const objs: string[] = ["device.o"];
        fs.copyFileSync(
            path.join(appDir, "device.cc"),
            path.join(appComponentDir, "device.cc"),
        );

        for (const plugin of Object.values(loadPlugins(config.plugins))) {
            if (plugin.sources) {
                for (const source of plugin.sources) {
                    fs.copyFileSync(
                        path.join(plugin.dir, source),
                        path.join(appComponentDir, source)
                    );

                    if (source.match(/\.(cc|cpp)$/)) {
                        objs.push(source.replace(/\..+$/, ".o"));
                    }
                }
            }
        }

        fs.writeFileSync(
            path.join(appComponentDir, "component.mk"), `\
COMPONENT_OBJS := ${objs.join(" ")}
CXXFLAGS += -fdiagnostics-color=always
`);

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
            path.join(esp32Dir, "build/firmware.elf"),
            path.join(appDir, "firmware.elf"),
        );
    }

    public async install(appDir: string, config: InstallConfig): Promise<void> {
        if (!config.serial) {
            throw new Error("serial is not set")
        }

        const esp32Dir = this.prepareEsp32Dir();
        const esptoolPath = path.resolve(esp32Dir, "deps/esp-idf/components/esptool_py/esptool/esptool.py");
        const tmpElfPath =  path.join(appDir, `firmware.${config.deviceName}.elf`);
        const rawFirmwarePath =  path.join(appDir, `firmware.${config.deviceName}.raw`);
        const firmwarePath =  path.join(appDir, `firmware.${config.deviceName}.bin`);
        const elf = fs.readFileSync(path.join(esp32Dir, "build/firmware.elf"));
        fs.writeFileSync(tmpElfPath, prepareFirmware(elf, config));

        spawnSync("python", [
            esptoolPath,
            '--chip', 'esp32',
            'elf2image',
            '--flash_mode', 'dio',
            '--flash_freq', '40m',
            '--flash_size', '2MB',
            '-o', rawFirmwarePath,
            tmpElfPath
        ]);


        const image = removeFirmwareHeader(createFirmwareImage(fs.readFileSync(rawFirmwarePath)));
        fs.writeFileSync(firmwarePath, image);

        const args = [
            esptoolPath,
            "--port", config.serial,
            "--baud", "921600",
            "--chip", "esp32",
            "--before", "default_reset", "--after", "hard_reset" ,
            "write_flash",
            "-z", "--flash_mode", "dio", "--flash_freq", "80m", "--flash_size", "detect",
            "0x8000", "default.bin",
            "0xe000", "otadata.bin",
            "0x1000", "bootloader/bootloader.bin",
            "0x10000", firmwarePath,
        ]

        // Create otadata.bin to initialize the otadata partiton.
        fs.writeFileSync(
            path.join(esp32Dir, "build/otadata.bin"),
            Buffer.alloc(0x2000 /* The size of otadata partition */)
        );

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
