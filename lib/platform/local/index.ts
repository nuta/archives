import { LocalPlatform } from "./runtime";
import { logger } from "../../logger";
import * as path from "path";
import * as net from "net";


export async function command(deviceName: string, command: string, arg: string) {
    return new Promise((resolve, reject) => {
        const sock = net.createConnection(path.join("dev.sock"));

        sock.on("error", (error: any) => {
            sock.destroy();
            if (error.code === "ENOENT") {
                reject(new Error("Failed to connect to the dev server. Run dev command."));
            } else {
                reject(error);
            }
        })

        sock.on("connect", () => {
            sock.write(JSON.stringify({
                type: "command",
                command: {
                    device: deviceName,
                    name: command,
                    arg,
                }
            }))

            sock.on("data", (data) => {
                const resp = JSON.parse(data.toString("utf-8"));
                sock.destroy();

                if (resp.result !== "success") {
                    reject(new Error(`server returned an error`));
                }

                resolve(resp);
            })
        })
    })
}

export async function viewLog() {
    logger.error("log command is not available in local platform")
}


export const Platform = LocalPlatform;
