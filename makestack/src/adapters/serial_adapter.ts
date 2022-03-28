import * as SerialPort from "serialport";
import { bufferReplace } from "../helpers";
const chalk = require("chalk");
const SerialPortDelimiter = require("@serialport/parser-delimiter");

const PACKET_DELIMITER = Buffer.from([0x0a]);
const PACKET_MAGIC = Buffer.from([0xe7, 0x4d, 0x50, 0x4b, 0x54]);

function encodePacket(payload: Buffer): Buffer {
    // Escape sequences.
    payload = bufferReplace(
        payload,
        Buffer.from([0xee]),
        Buffer.from([0xee, 0xe1])
    );
    payload = bufferReplace(
        payload,
        Buffer.from([0x0a]),
        Buffer.from([0xee, 0xe0])
    );

    const delimiter = Buffer.from([0x0a]);
    return Buffer.concat([delimiter, PACKET_MAGIC, payload, delimiter]);
}

function decodePacket(packet: Buffer): Buffer {
    const magic = packet.slice(0, PACKET_MAGIC.length);
    if (!magic.equals(PACKET_MAGIC)) {
        throw new Error(`invalid magic (${magic})`);
    }

    let payload = packet.slice(PACKET_MAGIC.length);
    payload = bufferReplace(
        payload,
        Buffer.from([0xee, 0xe0]),
        Buffer.from([0x0a])
    );
    payload = bufferReplace(
        payload,
        Buffer.from([0xee, 0xe1]),
        Buffer.from([0xee])
    );
    return payload;
}

export class SerialAdapter {
    private serial?: SerialPort;

    public open(
        port: string,
        baudRate: number,
        onRecv: (payload: Buffer) => void
    ): Promise<void> {
        return new Promise((resolve, reject) => {
            this.serial = new SerialPort(port, { baudRate });
            this.serial.on("error", reject);
            this.serial.on("open", () => {
                const serialLines = this.serial!.pipe(
                    new SerialPortDelimiter({ delimiter: PACKET_DELIMITER })
                );
                serialLines.on("data", (line: Buffer) => {
                    if (
                        line.slice(0, PACKET_MAGIC.length).equals(PACKET_MAGIC)
                    ) {
                        onRecv(decodePacket(line));
                    } else {
                        // Log message.
                        console.log(
                            chalk.magenta(`> ${line.toString("ascii").trim()}`)
                        );
                    }
                });

                resolve();
            });
        });
    }

    public send(payload: Buffer) {
        this.serial!.write(encodePacket(payload));
    }
}
