import { fork, spawnSync } from "child_process";
import * as crypto from "crypto";
import * as fs from "fs";
import * as os from "os";
import * as path from "path";
import * as util from "util";
import * as vm from "vm";
import * as fsutils from "./fsutils";
import { verifyImageHMAC, verifyMessageHMAC } from "./hmac";
import * as logger from "./logger";
import { deserialize, serialize } from "./smms";
import * as unzip from "./unzip";
import { Configs, DeviceState, PayloadMessages } from "./types";
import { AdapterBase } from "./adapters/adapter_base";
import * as apis from "makestack";
import { HTTPAdapter } from "./adapters/http_adapter";
import { SakuraIOAdapter } from "./adapters/sakuraio_adapter";

export interface SupervisorConstructorArgs {
    adapter: {
        name: string,
        url?: string,
    };
    mode: 'production' | 'debug' | 'test';
    appDir: string;
    osVersion: number;
    deviceId: string;
    deviceSecret: string;
    appUID?: number;
    appGID?: number;
    heartbeatInterval: number;

    // For those who run `./sdk/bin/makestack run` in the git repo.
    appNodePath?: string;
}

function osVersionToString(version: number) {
    const major = (version >> 16) & 0xff
    const minor = (version >> 8) & 0xff
    const patch = (version >> 0) & 0xff
    return `v${major}.${minor}.${patch}`
}

export class Supervisor {
    private app: any;
    private appDir: string;
    private currentAppDir: string;
    private osVersion: number;
    private mode: string;
    private debugMode: boolean;
    private testMode: boolean;
    private appUID?: number;
    private appGID?: number;
    private heartbeatInterval: number;
    private deviceId: string;
    private deviceSecret: string;
    private deviceType: string;
    private device: any;
    private appVersion: number;
    private log: string;
    private allLog: string;
    private configs: any;
    private adapterName: string;
    private updateEnabled: boolean;
    private downloading: boolean;
    private adapter: AdapterBase;
    private includeDeviceId: boolean;
    private rebooting: boolean;
    private heartbeatTimer?: any;
    private appNodePath?: string;
    private onMakeStackLinux: boolean;

    constructor(args: SupervisorConstructorArgs) {
        if (args.mode !== 'test') {
            process.on("unhandledRejection", (reason: any) => {
                console.log("supervisor: unhandled rejection:", reason);
                console.log("supervisor: exiting...");
                process.exit(1);
            });
        }

        this.app = null;
        this.appDir = args.appDir;
        this.currentAppDir = path.join(args.appDir, "current");
        this.osVersion = args.osVersion;
        this.mode = args.mode;
        this.debugMode = args.mode === 'debug';
        this.testMode = args.mode === 'test';
        this.appUID = args.appUID;
        this.appGID = args.appGID;
        this.heartbeatInterval = args.heartbeatInterval || 15;
        this.deviceId = args.deviceId;
        this.deviceSecret = args.deviceSecret;
        this.deviceType = apis.Device.getDeviceType();
        const { Device } = require(`./devices/${this.deviceType}`);
        this.device = new Device();
        this.appVersion = 0;
        this.log = "";
        this.allLog = "";
        this.configs = {};
        this.adapterName = args.adapter.name;
        this.updateEnabled = true;
        this.downloading = false;
        this.rebooting = false;
        this.appNodePath = args.appNodePath;
        this.onMakeStackLinux = fs.existsSync('/VERSION');


        switch (this.adapterName) {
            case "http": {
                if (!args.adapter.url) {
                    throw new Error('adapter.url is not specified')
                }

                this.adapter = new HTTPAdapter(this.deviceType, this.deviceId, args.adapter.url);
                this.includeDeviceId = true;
                break;
            }
            case "sakuraio": {
                this.adapter = new SakuraIOAdapter();
                this.includeDeviceId = false;
                break;
            }
            default:
            throw new Error(`unknown adapter \`${this.adapterName}'`);
        }
    }

    public destroy() {
        if (this.heartbeatTimer) {
            clearInterval(this.heartbeatTimer);
        }
    }

    private popLog() {
        const log = this.log;
        this.log = "";
        return log;
    }

    private updateOS(image: Buffer) {
        logger.warn("OS will be updated soon!");

        // Wait the app to exit.
        setTimeout(() => {
            logger.info("updating os image...");
            this.device.updateOS(image);
            logger.warn("updated the os, rebooting...")
            this.reboot();
        }, 5000);
    }

    private killApp() {
        if (this.app) {
            logger.warn("sending SIGTERM to the app...");
            this.app.kill();
        }
    }

    private launchApp(appZip: Buffer) {
        fsutils.removeFiles(this.currentAppDir);
        unzip.extract(appZip, this.currentAppDir);
        this.spawnApp();
    }

    private spawnApp() {
        if (this.app) {
            logger.info("killing the app");

            this.app.on("exit", () => {
                if (this.rebooting) {
                    this.doReboot();
                }

                // This callback could called after 'error' event.
                // This guard is for preventing spawn an app twice.
                if (!this.app) {
                    this.doSpawnApp();
                }
            });

            this.app.on("error", () => {
                // This callback could called after 'error' event.
                // This guard is for preventing spawn an app twice.
                if (!this.app) {
                    this.doSpawnApp();
                }
            });

            this.app.kill();
        } else {
            this.doSpawnApp();
        }
    }

    private doSpawnApp() {
        logger.info("starting an app");
        const env = {}

        if (this.appNodePath) {
            Object.assign(env, {
                NODE_PATH: this.appNodePath
            })
        }

        this.app = fork("./start", [], {
            cwd: this.currentAppDir,
            stdio: "inherit",
            uid: this.appUID,
            gid: this.appGID,
            env
        } as any);
        this.sendToApp("initialize", { configs: this.configs });

        this.app.on("message", (data: { type: string, body: string }) => {
            switch (data.type) {
                case "log":
                    const line = data.body.replace(/[ \t\n]+$/, '');
                    logger.debug("log:", line);

                    const lineWithTime = (this.adapterName === 'http')
                        ? `=${Date.now() / 1000} ${line}`
                        : line;

                    this.log += lineWithTime + "\n";
                    if (this.testMode) {
                        this.allLog += lineWithTime + "\n";
                    }
                    break;
                case "setUpdateLock":
                    this.updateEnabled = (data.body !== 'lock');
                    break;
                default:
                    logger.warn("unknown message", data.type);
            }
        });

        this.app.on("exit", () => {
            this.app = null;
            logger.info("app exited");

            setTimeout(() => {
                if (!this.app) {
                    logger.info("restarting app...");
                    this.spawnApp();
                }
            }, 5000);
        });
    }

    private waitForApp() {
        return new Promise((resolve, reject) => {
            if (!this.app) {
                resolve(this.allLog);
                return;
            }

            this.app.on("exit",  () => { resolve(this.allLog); });
            this.app.on("error", () => { resolve(this.allLog); });
        });
    }

    private sendToApp(type: string, data: Object) {
        if (!this.app) {
            // The app is being killed.
            return;
        }

        this.app.send(Object.assign({ type }, data));
    }

    private async sendHeartbeat(state: DeviceState) {
        if (this.downloading) {
            return;
        }

        logger.info(`heartbeating (state=running, os_ver="${this.osVersion}", ` +
                    `app_ver="${this.appVersion}", mode=${this.mode})`);

        await this.adapter.send(serialize({
            deviceId: this.deviceId,
            log: this.popLog(),
            reports: {
                currentVersion: this.appVersion
            }
        }, {
            includeDeviceId: this.includeDeviceId,
            deviceSecret: this.deviceSecret
        }));
    }

    private async handleUpdateAppMessage(appVersion: number) {
        if (appVersion <= this.appVersion) {
            return;
        }

        logger.info(`updating ${this.appVersion} -> ${appVersion}`);
        this.appVersion = appVersion;

        let appZip;
        this.downloading = true;
        try {
            appZip = await this.adapter.getAppImage(appVersion);
        } catch (e) {
            logger.error("failed to download app image:", e);
            this.downloading = false;
            return;
        } finally {
            this.downloading = false;
        }

        this.launchApp(appZip);
    }

    private async handleUpdateOSMessage(osVersion: number) {
        logger.info(`updating OS ${osVersionToString(this.osVersion)} -> ${osVersionToString(osVersion)}`);

        if (!this.onMakeStackLinux) {
            logger.info("The OS is not MakeStack OS, ignoring os update request.")
            return
        }

        this.downloading = true;

        let image
        try {
            image = await this.adapter.getOSImage(osVersion)
        } catch (e) {
            logger.error("failed to download os image:", e);
            this.downloading = false;
            return;
        } finally {
            this.downloading = false;
        }

        this.updateOS(image);
    }

    private reboot() {
        this.rebooting = true;
        if (this.app) {
            setTimeout(() => {
                logger.warn("app does not exit, forcing reboot...");
                this.doReboot();
            }, 10 * 1000);

            this.killApp();
        } else {
            this.doReboot();
        }
    }

    private doReboot() {
        // /init script reboots the system if Supervisor exit with 0.
        logger.info("Received a reboot command. Exiting with 0...");
        process.exit(0);
    }

    private handleConfigMessage(configs: { [key: string]: string }) {
        this.configs = configs;
        this.sendToApp("configs", { configs });
    }

    private handleCommandMessage(commands: { [key: string]: string }) {
        this.sendToApp("commands", { commands });
    }

    private async onSMMSReceive(payload: Buffer) {
        const { commands, update, osupdate, configs } = deserialize(payload);

        // Update OS
        if (osupdate && this.updateEnabled && !this.downloading) {
            await this.handleUpdateOSMessage(osupdate.version);
            // handleUpdateOSMessage() returns if it ignores the update request.
        }

        // Update App
        if (update && this.updateEnabled && !this.downloading) {
            await this.handleUpdateAppMessage(update.version);
            return;
        }

        if (configs) {
            this.handleConfigMessage(configs);
        }

        if (commands) {
            this.handleCommandMessage(commands);
        }
    }

    public async doIntervalJob() {
        await this.sendHeartbeat("running");
    }

    public async start() {
        this.adapter.onReceive((payload: Buffer) => this.onSMMSReceive(payload));
        await this.adapter.connect();
        await this.sendHeartbeat("ready");

        this.heartbeatTimer = setInterval(() => {
            this.doIntervalJob()
        }, this.heartbeatInterval * 1000)
    }
}
