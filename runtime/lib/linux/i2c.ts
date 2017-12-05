import * as fs from "fs";
import { ioctl } from "../native";

const I2C_SLAVE = 0x0703;

export abstract class LinuxI2CAPI {
    public abstract path: string;
    public address: number;
    public fd: number;

    constructor({ address }) {
        this.address = address;
        this.fd = fs.openSync(this.path, "rs+");
    }

    public read(length) {
        this.selectSlaveAddress(this.address);
        const buffer = Buffer.alloc(length);
        fs.readSync(this.fd, buffer, 0, length, 0);
        return buffer;
    }

    public write(data) {
        this.selectSlaveAddress(this.address);
        fs.writeSync(this.fd, Buffer.from(data));
    }

    private selectSlaveAddress(address) {
        if (ioctl(this.fd, I2C_SLAVE, address) !== 0) {
            throw new Error("failed to set I2C_SLAVE");
        }
    }
}
