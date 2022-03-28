import {
    Args,
    Command,
    Opts,
    DEVICE_FILE_OPTS,
} from "./command";
import * as SerialPort from "serialport";
const SerialPortDelimiter = require("@serialport/parser-delimiter");

function printLines(device: string, baudRate: number): Promise<void> {
    return new Promise((resolve, reject) => {
        const serial = new SerialPort(device, { baudRate });
        serial.on("error", reject);
        serial.on("open", () => {
            const delimiter = Buffer.from([0x0a]);
            const serialLines = serial!.pipe(
                new SerialPortDelimiter({ delimiter })
            );
            serialLines.on("data", (line: Buffer) => {
                console.log(line.toString("ascii").replace(/\r?\n$/, ""));
            });

            resolve();
        });
    });
}

export class SerialCommand extends Command {
    public static command = "serial";
    public static desc = "Read device log messages from the serial port.";
    public static args = [];
    public static opts = [
        ...DEVICE_FILE_OPTS,
    ];

    public async run(args: Args, opts: Opts) {
        await printLines(opts.device, opts.baudrate);
    }
}
