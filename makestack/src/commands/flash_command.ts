import {
    Args,
    Command,
    Opts,
    DEVICE_FILE_OPTS,
    BUILD_OPTS,
} from "./command";
import { Board, BuildOptions } from "../boards";
import { buildApp, transpileApp } from "../firmware";
import { logger } from "../logger";

export class FlashCommand extends Command {
    public static command = "flash";
    public static desc = "Install the firmware on a connected device.";
    public static args = [];
    public static opts = [
        ...BUILD_OPTS,
        ...DEVICE_FILE_OPTS,
        {
            name: "--wifi-ssid <ssid>",
            desc: "The Wi-Fi SSID.",
        },
        {
            name: "--wifi-password <password>",
            desc: "The Wi-Fi password (you can set this value by $WIFI_PASSWORD environment value instead).",
            default: "",
        },
        {
            name: "--server-url <url>",
            desc: "The server URL. (e.g., https://us-somewhere1-XXXXX.cloudfunctions.net for firebase)." +
                `We strongly recommend to use HTTPS (i.e., URLs starting with "https://") if possible.`
        }
    ];

    public async run(_args: Args, opts: Opts) {
        if (opts.adapter == "wifi") {
            if (!opts.wifiSsid) {
                logger.error("The --wifi-ssid option is required.");
            }

            opts.wifiPassword = opts.wifiPassword || process.env.WIFI_PASSWORD;
            if (!opts.wifiPassword) {
                logger.error("The --wifi-password is required.");
            }

            if (!opts.serverUrl) {
                logger.error("The --server-url is required.");
            }
        }

        logger.progress("Flashing...");
        const appCxx = transpileApp(opts.appDir);
        await opts.board.flashFirmware(opts.appDir, appCxx, opts.device, opts as BuildOptions);
        logger.success("Done!");
    }
}
