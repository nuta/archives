import * as SerialPort from "serialport";
import { guessSerialFilePath } from "../../helpers";
import { logger } from "../../logger";
import { AdapterCallback, Plugin } from "../../plugins";
import { parseVariableLength } from "../../telemata";

const PACKET_HEADER = Buffer.from([0xaa, 0xab, 0xff]);

async function sleep(ms: number): Promise<void> {
    return new Promise<void>((resolve, reject) => {
        setTimeout(() => resolve(), ms);
    });
}

export default class SerialAdapter extends Plugin {
    private serial?: SerialPort;
    constructor() {
        super();

        const serialFilePath = process.env.SERIAL || guessSerialFilePath();
        if (!serialFilePath) {
            logger.warn("Failed to open a serial device file. Disabling the serial adapter plugin.");
            return;
        }

        this.serial = new SerialPort(serialFilePath, { baudRate: 115200 });
    }

    public async sendChunk(chunk: Buffer): Promise<void> {
        return new Promise<void>((resolve, reject) => {
            if (!this.serial) {
                resolve();
                return;
            }

            this.serial.write(chunk, (error, len) => {
                if (error) {
                    console.error("serialport returned error:", error);
                    reject(error);
                } else {
                    resolve();
                }
            });
        });
    }

    public async sendPayload(payload: Buffer): Promise<void> {
        const data = Buffer.concat([PACKET_HEADER, payload]);

        let i = 0;
        const CHUNK_SIZE = 2048;
        while (true) {
            const chunk = data.slice(i * CHUNK_SIZE, (i + 1) * CHUNK_SIZE);
            if (chunk.length === 0) {
                break;
            }

            await this.sendChunk(chunk);
            if (i === 0) {
                await sleep(3000);
            }// ota_begin takes long
            await sleep(300);
            i++;
        }
    }

    public receivePayload(callback: AdapterCallback) {
        if (!this.serial) {
            return;
        }

        let buf = Buffer.alloc(0);
        this.serial.on("data", (data: Buffer) => {
            buf = Buffer.concat([buf, data]);

            const offset = buf.indexOf(PACKET_HEADER);
            if (offset >= 0) {
                const start = offset + PACKET_HEADER.length;
                const [length, lengthLength] = parseVariableLength(buf.slice(start));
                const end = start + lengthLength + length;
                const payload = buf.slice(start, end);
                callback(payload);
                buf = buf.slice(end);
            } else {
                process.stdout.write(buf);
                buf = Buffer.alloc(0);
            }
        });
    }
}
