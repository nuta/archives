export const ioctl: (fd: number, request: number, data: Buffer | number) => number = require(`../native/${process.arch}/ioctl.node`).ioctl;
export const spiConfigure: (fd: number, mode: number, bits: number, speed: number, order: number) => void = require(`../native/${process.arch}/spi.node`).configure;
export const spiTransfer: (fd: number, speed: number, tx: Buffer, rx: Buffer) => number = require(`../native/${process.arch}/spi.node`).transfer;
export const serialConfigure: (fd: number, baudrate: number, databits: number, parity: number) => void = require(`../native/${process.arch}/serial.node`).configure;
