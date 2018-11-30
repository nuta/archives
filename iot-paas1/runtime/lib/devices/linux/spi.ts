import * as fs from "fs";
import { functions as native } from "../../native";
import { GPIO } from "../..";
import {
    SPIMode, SPIOrder
} from "../../types";

const SPI_CPHA = 0x01;
const SPI_CPOL = 0x02;
const SPI_MODES: { [key: string]: number } = {
    MODE0: 0x00,
    MODE1: SPI_CPHA,
    MODE2: SPI_CPOL,
    MODE3: SPI_CPHA | SPI_CPOL,
};

export abstract class LinuxSPIAPI {
    public bus: string;
    public ss?: any;
    public fd: number;
    public mode: SPIMode;
    public bits: number;
    public speed: number;
    public order: SPIOrder;

    constructor(bus: string, args: any) {
        this.bus = bus;

        let path;
        const modeNumber = SPI_MODES[args.mode];
        if (typeof modeNumber !== "number") {
            throw new Error("invalid spi mode");
        }

        if (args.slave) {
            path = `/dev/spidev${bus}.${args.slave}`;
        } else {
            if (!args.path) {
                throw new Error("Specify `path' or `slave'.");
            }

            path = args.path;
        }

        if (args.ss !== undefined) {
            if (!GPIO) {
                throw new Error("GPIO API is unavailable.");
            }

            this.ss = new GPIO({ pin: args.ss, mode: 'out' });
            this.deselectSlave();
        } else {
            // Chip Select (or Slave Select) is controlled by the kernel
            this.ss = undefined;
        }

        this.mode = args.mode || "MODE0";
        this.bits = args.bits || 8;
        this.speed = args.speed || 500000;
        this.order = args.order || "MSBFIRST";
        this.fd = fs.openSync(path, "rs+");

        native.spiConfigure(
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
            native.spiTransfer(this.fd, this.speed, Buffer.from(tx), rx);
        } finally {
            this.deselectSlave();
        }
        return rx;
    }
}
