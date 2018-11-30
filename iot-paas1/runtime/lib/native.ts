import * as fs from "fs";
import * as path from "path";
import { getDeviceType } from "./helpers";

export interface NativeFunctions {
    ioctl: (fd: number, request: number, data: Buffer | number) => number;
    spiConfigure: (fd: number, mode: number, bits: number, speed: number, order: number) => void;
    spiTransfer: (fd: number, speed: number, tx: Buffer, rx: Buffer) => number;
    serialConfigure: (fd: number, baudrate: number, databits: number, parity: number) => void;
};

export const functions: NativeFunctions = require(
    path.resolve(__dirname, '../build/Release/native.node')
);
