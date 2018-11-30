import * as fs from "fs";
import { functions as native } from "../native";
const { O_RDWR, O_NOCTTY, O_SYNC } = fs.constants;

export class SerialAPI {
    public path: string;
    public watching: boolean;
    public fd: number;
    public baudrate: number;

    constructor(args: { path: string, baudrate: number }) {
        if (!args.baudrate) {
            throw new Error("`baudrate' is not speicified");
        }

        this.path = args.path;
        this.watching = false;
        this.baudrate = args.baudrate;
        this.fd = fs.openSync(args.path, O_RDWR | O_NOCTTY | O_SYNC);
        this.configure(args.baudrate);
    }

    public static list() {
        return fs.readdirSync("/dev")
            .filter((filepath) => filepath.match(/^(ttyAMA0|ttyUSB)/))
            .map((filepath) => `/dev/${filepath}`);
    }

    public configure(baudrate: number) {
        native.serialConfigure(this.fd, baudrate, 0, 0);
    }

    public writeSync(data: Buffer) {
        fs.writeSync(this.fd, data);
    }

    public readSync() {
        return fs.readFileSync(this.fd);
    }

    public onData(callback: (chunk: Buffer) => void) {
        if (this.watching) {
            throw Error("The serial port is already being watched.");
        }

        this.watching = true;

        // FIXME: use libuv
        setInterval(() => {
            const chunk = this.readSync();
            if (chunk.length > 0) {
                callback(chunk);
            }
        }, 100);
    }

    public onNewLine(callback: (line: string) => void) {
        if (this.watching) {
            throw Error("The serial port is already being watched.");
        }

        this.watching = true;

        // FIXME: use libuv
        let buf = "";
        setInterval(() => {
            const chunk = this.readSync().toString("utf-8");
            if (chunk.length > 0) {
                buf += chunk;
                while (buf.includes("\n")) {
                    const line = buf.split("\n")[0].replace("\r$", "");
                    buf = buf.slice(buf.indexOf("\n") + 1);
                    callback(line);
                }
            }
        }, 100);
    }
}
