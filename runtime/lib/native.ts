import { getDeviceType } from "./helpers";

export interface NativeFunctions {
    ioctl: (fd: number, request: number, data: Buffer | number) => number;
    spiConfigure: (fd: number, mode: number, bits: number, speed: number, order: number) => void;
    spiTransfer: (fd: number, speed: number, tx: Buffer, rx: Buffer) => number;
    serialConfigure: (fd: number, baudrate: number, databits: number, parity: number) => void;
};

console.log(process.env.MAKESTACK_ENV)
const nativeModulePath =
    (getDeviceType() === 'sdk' || process.env.MAKESTACK_ENV === 'test') ?
        '../build/Release/native' : `../native/${process.arch}/native.node`;

export const functions: NativeFunctions = require(nativeModulePath);
