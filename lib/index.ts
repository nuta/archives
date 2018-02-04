import { Serial } from "./serial";
import { CP2102Driver } from "./webusb/cp2102";
import { NodeSerialDriver } from "./node";
import { resolve } from "path";
import { setTimeout } from "timers";

const FLASH_WRITE_SIZE = 0x400;
const FLASH_SECTOR_SIZE = 0x1000;
const SECTORS_PER_BLOCK = 16;

export enum Command {
    FLASH_BEGIN = 0x02,
    FLASH_DATA = 0x03,
    FLASH_END = 0x04,
    SYNC = 0x08
}

export interface CommandResult {
    command: Command;
    data: Buffer;
}

export interface CommandOptions {
    timeout: number;
}

export interface Image {
    address: number;
    data: Buffer;
}

function replaceInBuffer(buf: Buffer, from: Buffer, to: Buffer): Buffer {
    let index = 0;
    while (true) {
        index = buf.indexOf(from, index);
        if (index === -1) {
            break;
        }

        buf = Buffer.concat([
            buf.slice(0, index),
            to,
            buf.slice(index + to.length)
        ])

        index += to.length;
    }

    return buf
}

function parseResponse(buf: Buffer): CommandResult | null {
    if (buf.length < 8) {
        return null;
    }

    // https://github.com/espressif/esptool/wiki/Serial-Protocol#response
    const type = buf.readUInt8(0)
    const command = buf.readUInt8(1)
    const size = buf.readUInt16LE(2)
    const data = buf.slice(8)

    if (type !== 0x01) {
        return null;
    }

    return {
        command, data
    }
}

function computeEraseSize(address: number, size: number): number {
    // TODO: Handle a bug in the ROM. Refer:
    // https://github.com/igrr/esptool-ck/blob/c69b9525b6a659b4e835075cba8c1f418c5b878a/espcomm/espcomm.c#L357

    return size;
}

export class Esptool {
    path: string;
    baudRate: number;
    serial: Serial;


    constructor({ driver, path, baudRate }: { driver: "node" | "cp2102", path: string, baudRate: number }) {

        this.path = path;
        this.baudRate = baudRate;

        switch (driver) {
            case "node":
            this.serial = new NodeSerialDriver()
            break
            case "cp2102":
            this.serial = new CP2102Driver()
            break
            default:
            throw new Error(`unknown serial port driver \`${driver}'`)
        }
    }

    open() {
        return this.serial.open({
            path: this.path,
            baudRate: this.baudRate
        })
    }

    async sendFrame(data: Buffer) {
        // Handle SLIP escape sequences.
        data = replaceInBuffer(data, Buffer.from([0xdb]), Buffer.from([0xdb, 0xdd]))
        data = replaceInBuffer(data, Buffer.from([0xc0]), Buffer.from([0xdb, 0xdc]))

        await this.serial.write(Buffer.concat([
            Buffer.from([0xc0]),
            data,
            Buffer.from([0xc0])
        ]))
    }

    // Find a frame end of Serial Line Internet Protocol.
    async readFrame(): Promise<Buffer> {
        /* Read until a SLIP packet header. */
        while (true) {
            const byte = (await this.serial.read(1))[0];

            if (byte === 0xc0) {
                break;
            }
        }

        let buf = Buffer.alloc(0);
        while (true) {
            const byte = (await this.serial.read(1))[0];

            if (byte === 0xc0) {
                // End of packet.
                break;
            }

            buf = Buffer.concat([buf, Buffer.from([byte])]);
        }

        console.log('readframe!');
        // Handle SLIP escape sequences.
        buf = replaceInBuffer(buf, Buffer.from([0xdb, 0xdd]), Buffer.from([0xdb]));
        buf = replaceInBuffer(buf, Buffer.from([0xdb, 0xdc]), Buffer.from([0xc0]));
        console.log('returning readframe!');

        return buf;
    }

    async doCommand(command: Command, data: Buffer, options?: CommandOptions): Promise<CommandResult> {
        const header = Buffer.alloc(8)
        header.writeUInt8(0x00, 0); // request
        header.writeUInt8(command, 1);
        header.writeUInt16LE(data.length, 2);
        header.writeUInt32LE(0, 4); // checksum

        console.log('sending a frame');
        await this.sendFrame(Buffer.concat([header, data]));
        console.log('sent frame')

        for (let i = 0; i < 100; i++) {
            console.log('receiving response');
            const resp = parseResponse(await this.readFrame());
            if (resp === null || resp.command !== command) {
                console.log(resp);
                // Retry
                continue;
            }

            console.log('readframe done');
            return resp;
        }

        throw new Error("Failed to read the command response.")
    }

    command(command: Command, data: Buffer, options?: CommandOptions): Promise<CommandResult | null> {
        console.log(`command: type=0x${command.toString(16)}, length=${data.length}`)
        console.log(data)

        return new Promise((resolve, reject) => {
            let timer: any;
            if (options) {
                timer = setTimeout(() => {
                    resolve(null)
                }, options.timeout)
            }

            this.doCommand(command, data, options).then(resp => {
                if (timer) {
                    clearTimeout(timer);
                }

                resolve(resp);
            })
        })
    }

    async sync() {
        const syncData = Buffer.from([
            0x07, 0x07, 0x12, 0x20,
            0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
            0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
            0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
            0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55
        ])

        const r = await this.command(Command.SYNC, syncData, { timeout: 500 })
        return (r !== null);
    }

    async flashBegin(address: number, size: number) {
        const numBlocks = Math.ceil(size / FLASH_WRITE_SIZE);
        const payload = Buffer.alloc(16);
        payload.writeUInt32LE(computeEraseSize(address, size), 0);
        payload.writeUInt32LE(numBlocks, 4);
        payload.writeUInt32LE(FLASH_WRITE_SIZE, 8);
        payload.writeUInt32LE(address, 12);
        await this.command(Command.FLASH_BEGIN, payload);
    }

    async flashBlock(seq: number, chunk: Buffer) {
        const header = Buffer.alloc(16);
        header.writeUInt32LE(chunk.length, 0);
        header.writeUInt32LE(seq, 4);
        header.writeUInt32LE(0, 8);
        header.writeUInt32LE(0, 12);

        const payload = Buffer.concat([header, chunk]);
        await this.command(Command.FLASH_DATA, payload);
    }

    async flashEnd(reboot: boolean = false) {
        const payload = Buffer.alloc(4);
        payload.writeUInt32LE(reboot ? 0 : 1, 0);
        await this.command(Command.FLASH_END, payload);
    }

    async flashImage(address: number, data: Buffer, reboot: boolean = false) {
        console.log('>>> Erasing flash');
        await this.flashBegin(address, data.length);

        let seq = 0;
        while (data.length > 0) {
            console.log(`>>> Writing at 0x${address.toString(16)}`);
            const chunk = data.slice(0, FLASH_WRITE_SIZE);
            const padding = Buffer.alloc(FLASH_WRITE_SIZE - chunk.length, 0xff);

            await this.flashBlock(seq, Buffer.concat([chunk, padding]));

            data = data.slice(FLASH_WRITE_SIZE);
            seq++;
            address += FLASH_WRITE_SIZE;
        }

        console.log('>>> Finishing');
        await this.flashEnd(reboot);
    }

    async connect() {
        console.log('syncing...')
        for (let i = 0; i < 10; i++) {
            console.log(`${(i)}/10: connecting ***********************`)

            // TODO: reset to bootloader

            if (await this.sync()) {
                this.serial.flushInputBuffer();
                return;
            }
        }

        throw new Error("Failed to sync.");
    }

    async flash(files: [Image]) {
        console.log('connecting...')
        await this.connect()

        // TODO: Support bootloader stub.

        console.log('flashing...')
        let lastAddress = files[files.length - 1].address;
        for (const { address, data } of files) {
            await this.flashImage(address, data, address === lastAddress);
        }

        console.log('flashed!')
        this.serial.close()
    }
}

/* For web browsers. */
declare global {
    interface Window {
        Esptool: typeof Esptool
    }
}

if (typeof window !== "undefined") {
    window.Esptool = Esptool;
}
