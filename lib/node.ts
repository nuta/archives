import * as SerialPort from "serialport";
import { Serial } from "./serial";

export class NodeSerialDriver implements Serial {
    port: SerialPort;
    received: Buffer;
    tryReadTimer: any;

    constructor() {
        this.tryReadTimer = null;
        this.received = Buffer.alloc(0);
    }

    open(args: { path: string, baudRate: number }): Promise<void> {
        return new Promise((resolve, rejct) => {
            console.log(args)
            this.port = new SerialPort(args.path, {
                parity: 'none',
                stopBits: 1,
                baudRate: args.baudRate
            })

            this.port.on('open', () => resolve())
            this.port.on('error', e => console.error(`serialport: ${e}`))
            this.port.on('data', (data: Buffer) => {
                console.log('received:', data)
                this.received = Buffer.concat([this.received, data])
            })
        })
    }

    close() {
        if (this.tryReadTimer) {
            clearTimeout(this.tryReadTimer);
        }

        this.port.close()
    }

    read(length: number): Promise<Uint8Array> {
        return new Promise((resolve, reject) => {
            const tryRead = () => {
                if (this.tryReadTimer) {
                    clearTimeout(this.tryReadTimer);
                    this.tryReadTimer = null;
                }
                console.log(this.received);

                if (this.received.length >= length) {
                    const ret = this.received.slice(0, length);
                    this.received = this.received.slice(length);
                    resolve(new Uint8Array(ret));
                } else {
                    this.tryReadTimer = setTimeout(() => {
                        console.log(`try reading`);
                        tryRead();
                    }, 100)
                }
            }

            tryRead()
        })
    }

    async flushInputBuffer(): Promise<void> {
        this.received = Buffer.alloc(0);
    }

    write(data: Uint8Array): Promise<void> {
        return new Promise((resolve, reject) => {
            console.log('write:', data)
            this.port.write(new Buffer(data), (error, written) => {
                if (error) {
                    reject(error)
                } else {
                    resolve()
                }
            })
        })
    }
}
