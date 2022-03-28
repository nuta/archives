import * as fs from "fs";
import * as path from "path";
import * as express from "express";
import {
    Args,
    Command,
    Opts,
    APP_OPTS,
    BOARD_OPTS,
    BUILD_OPTS,
    ADAPTER_OPTS,
    validateDeviceFilePath,
} from "./command";
import { Board, BuildError, BuildOptions } from "../boards";
import { logger } from "../logger";
import { buildApp } from "../firmware";
import { SerialAdapter, WiFiAdapter } from "../adapters";
import { DevServer } from "../dev_server";
import { ProtocolServer } from "../server/server";
import { UserError } from "../helpers";

export class DevCommand extends Command {
    public static command = "dev";
    public static desc = "Starts up the app with auto update.";
    public static args = [];
    public static opts = [
        ...APP_OPTS,
        ...BOARD_OPTS,
        ...BUILD_OPTS,
        ...ADAPTER_OPTS,
        {
            name: "--device <path>",
            desc: "The device file path.",
        },
        {
            name: "--baudrate <rate>",
            desc: "The baudrate.",
            default: 115200,
        },
        {
            name: "--host <host>",
            desc: "The dev server hostname.",
            default: "0.0.0.0",
        },
        {
            name: "--port <port>",
            desc: "The dev server port.",
            default: 1234,
        },
    ];
    public static watchMode = true;

    private board!: Board;
    private devServer!: DevServer;
    private protocolServer!: ProtocolServer;

    public async run(_args: Args, opts: Opts) {
        this.board = opts.board;

        // First, build the firmware. We need the firmware file in order to send
        // the latest version info in a heartbeat.
        if (!(await this.build(opts.appDir, opts as BuildOptions))) {
            logger.error("fix build errors and run the command again");
            process.exit(1);
        }

        const httpServerPort = opts.port + 1;
        const httpServer = express();
        new WiFiAdapter(httpServer, payload => {
            let reply = this.protocolServer.processPayload(payload);
            return reply ? reply : this.protocolServer.buildHeartbeatPayload();
        });
        httpServer.listen(httpServerPort, "127.0.0.1");

        logger.progress("Initializing the adapter...");
        await this.initializeAdapter(opts.adapter, opts);

        logger.progress(`Starting a app...`);
        this.devServer = new DevServer(opts.host, opts.port, httpServerPort, opts.appDir);
        logger.progress(`Listen on ${opts.host}:${opts.port}`);

        // Watch for the app source files.
        fs.watch(opts.appDir, async (_event: string, filename: string) => {
            const appFile = path.join(opts.appDir, "app.js");
            if (filename == "app.js" && fs.existsSync(appFile)) {
                logger.progress("Change detected, restarting and rebuilding the app...");
                this.devServer.restart();
                await this.build(opts.appDir, opts as BuildOptions);
            }
        });

        logger.success(
            `We're ready! Watching for changes on ${opts.appDir}...`
        );
    }

    private async build(appDir: string, opts: BuildOptions) {
        logger.progress("Building the firmware...");
        try {
            await buildApp(this.board, appDir, opts);
        } catch (e) {
            if (e instanceof BuildError) {
                logger.error("failed to build");
                return false;
            } else {
                throw e;
            }
        }

        this.protocolServer = new ProtocolServer(
            this.board.getFirmwarePath(),
            (name: string, value: any) => {
            this.devServer.sendRequest({ type: "event", name, value });
        });

        logger.success("Build succeeded");
        return true;
    }

    private async initializeAdapter(adapter: string, opts: any) {
        switch (adapter) {
            case "serial":
                // FIXME:
                const device = (validateDeviceFilePath as any)(opts.device || "");

                const serialAdapter = new SerialAdapter();
                await serialAdapter.open(device, opts.baudrate, payload => {

                    const reply = this.protocolServer.processPayload(payload);
                    if (reply) {
                        serialAdapter.send(reply);
                    }
                });

                // Send heartbeats regularly.
                // TODO: Disable heartbeaing on firmware updating.
                serialAdapter.send(this.protocolServer.buildHeartbeatPayload());
                setInterval(() => {
                    serialAdapter.send(this.protocolServer.buildHeartbeatPayload());
                }, 2000);

                // Make sure that connected device is running the our firmware.
                setTimeout(() => {
                    if (!this.protocolServer.verifiedPong) {
                        logger.error(
                            "The device doesn't respond our health check.\nHint: Run `makestack flash --dev` to install the firmware."
                        );
                        process.exit(1);
                    }
                }, 5000);
                break;
            case "wifi":
                /* Already running. */
                break;
            default:
                throw new UserError(`Unknown adapter type: \`${adapter}'`);
        }
    }
}
