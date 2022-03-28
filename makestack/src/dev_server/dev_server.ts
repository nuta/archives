import * as path from "path";
import * as child_process from "child_process";

export interface DeviceEventMessage {
    type: "event",
    name: string,
    value: any,
}

export type DevServerRequest = DeviceEventMessage;

export class DevServer {
    private appDir: string;
    private host: string;
    private port: number;
    private devServerPort: number;
    private proc!: child_process.ChildProcess;

    constructor(host: string, port: number, devServerPort: number, appDir: string) {
        this.appDir = appDir;
        this.host = host;
        this.port = port;
        this.devServerPort = devServerPort;
        this.start();
    }

    public start() {
        const startModule = path.resolve(__dirname, "./start");
        const forkOpts = {
            env: {
                HOST: this.host,
                PORT: this.port.toString(),
                DEV_SERVER_PORT: this.devServerPort.toString(),
            },
            cwd: this.appDir,
        };

        this.proc = child_process.fork(startModule, [], forkOpts);
        process.on("exit", () => {
            this.proc.kill();
        });
    }

    public restart() {
        this.proc.on("exit", () => {
            this.start();
        })

        this.proc.kill();
    }

    public sendRequest(msg: DevServerRequest) {
        this.proc.send(msg);
    }
}
