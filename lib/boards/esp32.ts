import { spawn, spawnSync, SpawnSyncReturns } from "child_process";
import * as ejs from "ejs";
import * as fs from "fs-extra";
import * as os from "os";
import * as path from "path";
import { createFirmwareImage, CREDENTIALS_DATA_TEMPLATE, embedCredentials } from "../firmware";
import { execCmd } from "../helpers";
import { logger } from "../logger";
import { loadPlugins } from "../plugins";
import { Board, InstallConfig } from "../types";
const Gauge = require("gauge");
const packageJson = require(path.resolve(__dirname, "../../package.json"));

function make(isReleaseBuild: boolean, version: number, esp32Dir: string): Promise<any> {
    return new Promise((resolve, reject) => {
        // Update src/component.mk to force rebuild to embed new APP_VERSION.
        execCmd(["touch", path.join(esp32Dir, "src/component.mk")]);

        const makeExecutable = "/usr/bin/make";
        const procs = os.cpus().length;
        const args = [`-j${procs}`];
        const opts = {
            cwd: esp32Dir,
            stdio: "pipe",
            env: {
                V: "1",
                MAKE: makeExecutable,
                PATH: process.env.PATH,
                MY_COMPONENTS: "app",
                RELEASE: isReleaseBuild ? "1" : "",
                APP_VERSION: version,
                FIRMWARE_VERSION: packageJson.version,
            },
        };

        const totalLines = spawnSync(makeExecutable, ["--dry-run"].concat(args), opts)
                                .stdout.toString("utf-8").split("\n").length;

        const gauge = new Gauge(process.stderr, { theme: "colorASCII" });
        gauge.show("", 0.);
        const cp = spawn(makeExecutable, args, opts);

        let stderr = "";
        cp.stderr.on("data", (data: Buffer) => {
            stderr += data.toString("utf-8");
        });

        let stdout = "";
        let currentLines = 0;
        cp.stdout.on("data", (data: Buffer) => {
            const str = data.toString("utf-8");
            const lines = str.split("\n");
            const newLines = lines.length - 1;
            currentLines += newLines;
            const perc = currentLines / totalLines;
            gauge.show(`${Math.round(perc * 100)}%`, perc);
            gauge.pulse((newLines > 0) ? "Building..." : undefined);
            stdout += str;
        });

        cp.on("exit", (status) => {
            gauge.disable();
            resolve({ status, stdout, stderr });
        });
    });
}

const COMPONENT_MK_TMPL = `\
COMPONENT_OBJS := <%= objs.join(" ") %>
CXXFLAGS += -fdiagnostics-color=always
`;

export class Esp32Board extends Board {
    private copySources(plugins: string[], appDir: string, appComponentDir: string): string[] {
        const sources = [
            path.join(appDir, "device.cc"),
        ];

        // Get a list of plugin soruce files.
        for (const plugin of Object.values(loadPlugins(plugins))) {
            if (plugin.sources) {
                for (const source of plugin.sources) {
                    sources.push(path.join(plugin.dir, source));
                }
            }
        }

        // Copy source files and fill objs.
        const objs = [];
        for (const source of sources) {
            const basename = path.basename(source);
            fs.copyFileSync(source, path.join(appComponentDir, basename));
            if (basename.match(/\.(cc|cpp)$/)) {
                objs.push(basename.replace(/\..+$/, ".o"));
            }
        }

        return objs;
    }

    private async doBuild(isReleaseBuild: boolean, version: number, esp32Dir: string) {
        const cp = await make(isReleaseBuild, version, esp32Dir);
        if (cp.status != 0) {
            if (cp.stderr.includes("No rule to make target `cores/esp32/libb64/cencode.o'")) {
                // FIXME: Try again because the build system is broken.
                const cp2 = await make(isReleaseBuild, version, esp32Dir);
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

    }

    private copyBuiltArtifacts(appDir: string, esp32Dir: string, version: number) {
        let image = fs.readFileSync(path.join(esp32Dir, "build/firmware.bin"));
        fs.mkdirpSync(path.join(appDir, "build"));
        fs.copyFileSync(path.join(esp32Dir, "build/firmware.bin"), path.join(appDir, "build/firmware.esp32.bin"));
        image = createFirmwareImage(version, image);
        fs.writeFileSync(path.join(appDir, "esp32.firmware"), image);
    }

    public async build(isReleaseBuild: boolean, version: number, repoDir: string, appDir: string): Promise<void> {
        const esp32Dir = path.join(repoDir, "esp32");
        const appComponentDir = path.join(esp32Dir, "app");
        const config = fs.readJsonSync(path.join(appDir, "package.json")).makestack;

        if (!fs.existsSync(path.join(esp32Dir, "deps"))) {
            execCmd(["./tools/download-dependencies"],  { cwd: esp32Dir });
        }

        // Copy app and plugin source files.
        fs.mkdirpSync(appComponentDir);
        const objs = this.copySources(config.plugins, appDir, appComponentDir);

        // Generate component.mk
        fs.writeFileSync(
            path.join(appComponentDir, "component.mk"),
            ejs.render(COMPONENT_MK_TMPL, { objs }),
        );

        await this.doBuild(isReleaseBuild, version, esp32Dir);
        this.copyBuiltArtifacts(appDir, esp32Dir, version);
    }

    public async install(repoDir: string, appDir: string, config: InstallConfig): Promise<void> {
        if (!config.serial) {
            throw new Error("serial is not set");
        }

        const esp32Dir = path.join(repoDir, "esp32");
        const firmwarePath =  path.join(appDir, "build/firmware.esp32.bin");
        const credentialsPath = path.join(appDir, `build/credentials.${config.deviceName}.bin`);
        fs.writeFileSync(credentialsPath, embedCredentials(CREDENTIALS_DATA_TEMPLATE, config));

        // Create otadata.bin to initialize the otadata partiton.
        fs.writeFileSync(
            path.join(esp32Dir, "build/otadata.bin"),
            Buffer.alloc(0x2000 /* The size of otadata partition */),
        );

        // The partition table (default.bin) must be the following layout:
        // # Name, Type, SubType, Offset, Size, Flags
        // nvs,data,nvs,0x9000,20K,
        // otadata,data,ota,0xe000,8K,
        // app0,app,ota_0,0x10000,1280K,
        // app1,app,ota_1,0x150000,1280K,
        // eeprom,data,153,0x290000,4K,
        // spiffs,data,spiffs,0x291000,1468K, (we use here to save credentials instead)
        //
        // Credentials (i.e. device name, wifi password, etc.) exists at 0x291000; don't
        // use the space for SPIFFS or move the position or the firmware is no longer able
        // to communicate with the server.
        //
        const argv = [
            "python",
            path.resolve(esp32Dir, "deps/esp-idf/components/esptool_py/esptool/esptool.py"),
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
            "0x291000", credentialsPath,
        ];

        // TODO: ensure that pyserial is installed
        execCmd(argv, { cwd: path.join(esp32Dir, "build") });
    }
}
