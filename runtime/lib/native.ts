import * as fs from "fs";
import * as path from "path";
import { getDeviceType } from "./helpers";

export interface NativeFunctions {
    ioctl: (fd: number, request: number, data: Buffer | number) => number;
    spiConfigure: (fd: number, mode: number, bits: number, speed: number, order: number) => void;
    spiTransfer: (fd: number, speed: number, tx: Buffer, rx: Buffer) => number;
    serialConfigure: (fd: number, baudrate: number, databits: number, parity: number) => void;
};

const nativeModulePaths = [
    path.resolve(__dirname, '../build/Release/native.node'),
    path.resolve(__dirname, `../native/${process.arch}/native.node`)
]

const nativeModulePath = nativeModulePaths.filter(modulePath => fs.existsSync(modulePath))[0];
if (!nativeModulePath) {
    throw new Error('Native moudle not found.')
}

export const functions: NativeFunctions = require(nativeModulePath);
