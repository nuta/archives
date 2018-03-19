import * as util from 'util';
import { logger } from './logger';
import {
    GPIOInterface, GPIOPinMode, GPIOInterruptMode,
    I2CInterface,
    SPIInterface, SPIMode, SPIOrder
} from "./types";

class dummyGPIO implements GPIOInterface {
    constructor(args: { pin: number, mode: GPIOPinMode }) {
        logger.warn(`new dummyGPIO(${args})`);
    }

    setMode(mode: GPIOPinMode): void {
        logger.warn(`dummyGPIO.setMode(${mode})`);
    }

    writeSync(value: boolean): void {
        logger.warn(`dummyGPIO.writeSync(${value})`);
    }

    readSync(): boolean {
        logger.warn('dummyGPIO.readSync() returning false');
        return false;
    }

    onInterrupt(mode: GPIOInterruptMode, callback: () => void): void {
        logger.warn(`dummyGPIO.onInterrupt(${mode}, ${util.inspect(callback)})`);
    }
}

class dummyI2C implements I2CInterface {
    constructor(args: { address: number }) {
        logger.warn(`new dummyI2C(${args})`);
    }

    readSync(length: number): Buffer {
        const retval = Buffer.alloc(length);
        logger.warn(`dummyI2C.readSync(${length}) returning ${util.inspect(retval)}`);
        return retval;
    }
    writeSync(data: number[] | Buffer) {
        logger.warn(`dummyI2C.writeSync(${data})`);
    }
}

class dummySPI implements SPIInterface {
    constructor(args: {
        slave: number,
        speed: number,
        mode: SPIMode,
        order: SPIOrder,
        bits: number,
        ss: number,
        path: string
    }) {
        logger.warn(`new dummySPI(${args})`);
    }

    transfer(tx: number[] | Buffer): Buffer {
        const retval = Buffer.alloc(tx.length);
        logger.warn(`dummySPI.transfer(${tx}) returning ${util.inspect(retval)}`);
        return retval;
    }
}

export { dummyGPIO, dummyI2C, dummySPI };
