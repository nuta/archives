import * as fs from "fs";
import { spiConfigure, spiTransfer } from "../native";

const SPI_CPHA = 0x01;
const SPI_CPOL = 0x02;
const SPI_MODES: { [key: string]: number } = {
    MODE0: 0x00,
    MODE1: SPI_CPHA,
    MODE2: SPI_CPOL,
    MODE3: SPI_CPHA | SPI_CPOL,
};

export abstract class LinuxSPIAPI {
    public abstract bus: string;
    public slave: number;
    public ss?: GPIO;
    public fd: number;
    public mode: SPIMode;
    public bits: number;
    public speed: number;
    public order: SPIOrder;

    constructor(args: { slave: number, speed: number, mode: SPIMode, order: SPIOrder, bits: number, ss: number, path: string }) {
        if (!args.path && !args.slave) {
            throw new Error("Specify `path' or `slave'.");
        }

        const path = (args.slave === undefined) ? args.path : `/dev/spidev${this.bus}.${args.slave}`;

        if (args.ss !== undefined) {
            this.ss = new GPIO({ pin: args.ss, mode: GPIO.OUTPUT });
            this.deselectSlave();
        } else {
            // Chip Select (or Slave Select) is controlled by the kernel
            this.ss = undefined;
        }

        this.fd = fs.openSync(path, "rs+");
        this.configure(args.mode, args.bits, args.speed, args.order);
    }

    public configure(mode: SPIMode, bits: number, speed: number, order: SPIOrder) {
        this.mode = mode || "MODE0";
        this.bits = bits || 8;
        this.speed = speed || 500000;
        this.order = order || "MSBFIRST";

        const modeNumber = SPI_MODES[this.mode];
        if (typeof modeNumber !== "number") {
            throw new Error("invalid spi mode");
        }

        spiConfigure(
            this.fd,
            modeNumber,
            this.bits,
            this.speed,
            (this.order === "LSBFIRST") ? 1 : 0,
        );
    }

    private selectSlave() {
        if (this.ss) {
            this.ss.write(false);
        }
    }

    private deselectSlave() {
        if (this.ss) {
            this.ss.write(true);
        }
    }

    public transfer(tx: Buffer) {
        const rx = Buffer.alloc(tx.length);
        this.selectSlave();
        try {
            spiTransfer(this.fd, this.speed, Buffer.from(tx), rx);
        } finally {
            this.deselectSlave();
        }
        return rx;
    }
}
