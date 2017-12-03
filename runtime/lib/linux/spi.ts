import * as fs from "fs";
const spi = require(`../../native/${process.arch}/spi.node`);

const SPI_CPHA = 0x01;
const SPI_CPOL = 0x02;
const SPI_MODES = {
    MODE0: 0x00,
    MODE1: SPI_CPHA,
    MODE2: SPI_CPOL,
    MODE3: SPI_CPHA | SPI_CPOL,
};

export abstract class LinuxSPIAPI {
    public abstract bus: string;
    public slave: number;
    public ss: GPIO;
    public fd: number;
    public mode: SPIMode;
    public bits: number;
    public speed: number;
    public order: SPIOrder;

    constructor({ path, ss, slave, speed, order, mode, bits }) {
        if (!path && !slave) {
            throw new Error("Specify `path' or `slave'.");
        }

        if (this.slave) {
            path = `/dev/spidev${this.bus}.${slave}`;
        }

        if (this.ss) {
            this.ss = new GPIO({ pin: ss, mode: GPIO.OUTPUT });
            this.deselectSlave();
        } else {
            // Chip Select (or Slave Select) is controlled by the kernel
            this.ss = undefined;
        }

        this.fd = fs.openSync(path, "rs+");
        this.configure(mode, bits, speed, order);
    }

    public configure(mode, bits, speed, order) {
        this.mode = mode || "MODE0";
        this.bits = bits || 8;
        this.speed = speed || 500000;
        this.order = order || "MSBFIRST";

        const modeNumber = SPI_MODES[this.mode];
        if (typeof modeNumber !== "number") {
            throw new Error("invalid spi mode");
        }

        spi.configure(
            this.fd,
            modeNumber,
            this.bits,
            this.speed,
            (this.order === "LSBFIRST") ? 1 : 0,
        );
    }

    public selectSlave() {
        if (this.ss) {
            this.ss.write(false);
        }
    }

    public deselectSlave() {
        if (this.ss) {
            this.ss.write(true);
        }
    }

    public transfer(tx) {
        const rx = Buffer.alloc(tx.length);
        this.selectSlave();
        try {
            spi.transfer(this.fd, this.speed, Buffer.from(tx), rx);
        } finally {
            this.deselectSlave();
        }
        return rx;
    }
}
