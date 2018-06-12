import * as SerialPort from "serialport";
import { Plugin, AdapterCallback } from "../../lib/plugins";
import { PackageConfig } from "../../lib/types";
import { parseVariableLength } from "../../lib/telemata";

const PACKET_HEADER = Buffer.from([0xaa, 0xab, 0xff]);

async function sleep(ms: number): Promise<void> {
    return new Promise<void>((resolve, reject) => {
        setTimeout(() => resolve(), ms);
    })
}

export default class SerialAdapter extends Plugin {
    private serial: SerialPort;
    constructor(config: PackageConfig) {
        super(config);

        config.serialFilePath = "/dev/cu.usbserial-14210";
        this.serial = new SerialPort(config.serialFilePath, {
            baudRate: 115200
        });
    }

    async sendChunk(chunk: Buffer): Promise<void> {
        return new Promise<void>((resolve, reject) => {
            this.serial.write(chunk, (error, len) => {
                if (error) {
                    console.error('serialport returned error:', error);
                    reject(error);
                } else {
                    resolve();
                }
            });
        })
    }

    async sendPayload(payload: Buffer): Promise<void> {
        const data = Buffer.concat([PACKET_HEADER, payload]);

        let i = 0;
        const CHUNK_SIZE = 2048;
        while (true) {
            const chunk = data.slice(i * CHUNK_SIZE, (i + 1) * CHUNK_SIZE);
            if (chunk.length == 0) {
                break;
            }

            console.log(`chunk ${i} ${chunk.length}`);
            await this.sendChunk(chunk);
            if (i == 0)
                await sleep(3000);// ota_begin takes long
            await sleep(300);
            i++;
        }
    }

    receivePayload(callback: AdapterCallback) {
        let buf = Buffer.alloc(0);
        this.serial.on('data', (data: Buffer) => {
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
        })
    }
}
