import * as net from "net";
import * as path from "path";
import { PlatformSdk } from "../sdk";

export class LocalPlatformSdk extends PlatformSdk {
    public async command(deviceName: string, command: string, arg: string): Promise<any> {
        return new Promise((resolve, reject) => {
            const sock = net.createConnection(path.join("dev.sock"));

            sock.on("error", (error: any) => {
                sock.destroy();
                if (error.code === "ENOENT") {
                    reject(new Error("Failed to connect to the dev server. Run dev command."));
                } else {
                    reject(error);
                }
            });

            sock.on("connect", () => {
                sock.write(JSON.stringify({
                    type: "command",
                    command: {
                        device: deviceName,
                        name: command,
                        arg,
                    },
                }));

                sock.on("data", (data) => {
                    const resp = JSON.parse(data.toString("utf-8"));
                    sock.destroy();

                    if (resp.result !== "success") {
                        reject(new Error(`server returned an error`));
                    }

                    resolve(resp);
                });
            });
        });
    }

    public async viewLog() {
        throw new Error("log command is not available in local platform");
    }

    public async deploy() {
        throw new Error("deploy command is not available in local platform");
    }
}
