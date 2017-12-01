export const ioctl: (fd: number, request: number, data: Buffer | number) => number = require(`../native/${process.arch}/ioctl.node`).ioctl
