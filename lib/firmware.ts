export function getFirmwareVersion(firmware: Buffer): number {
    return firmware.readUInt32LE(8);
}


export function createFirmwareImage(version: number, image: Buffer): Buffer {
    const firmwareHeader = Buffer.alloc(16);
    firmwareHeader.writeUInt8(0x81, 0);
    firmwareHeader.writeUInt8(0xf1, 1);
    firmwareHeader.writeUInt8(0x5c, 2);
    firmwareHeader.writeUInt8(0x5e, 3);
    firmwareHeader.writeUInt32LE(image.length, 4);
    firmwareHeader.writeUInt32LE(version, 8);
    return Buffer.concat([firmwareHeader, image]);
}
