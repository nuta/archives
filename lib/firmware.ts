import { InstallConfig } from "./types";

export function getFirmwareVersion(firmware: Buffer): number {
    return firmware.readUInt32LE(8);
}


function replaceBuffer(buf: Buffer, value: string, id: string, fill: number): Buffer {
    const needle = `__REPLACE_ME_${id}__`;

    const index = buf.indexOf(Buffer.from(needle));
    if (index === -1) {
        throw new Error(`replaceBuffer: failed to replace ${id}`);
    }

    const paddedValue = Buffer.alloc(needle.length, fill);
    const valueBuf = Buffer.from(value);
    valueBuf.copy(paddedValue);
    paddedValue.copy(buf, index);
    return buf;
}


export function removeFirmwareHeader(image: Buffer): Buffer {
    return image.slice(16);
}

export function prepareFirmware(image: Buffer, config: InstallConfig): Buffer {
    const fill = 0x20; /* a white space character */
    image = replaceBuffer(image, config.deviceName, "DEVICE_NAME", fill);
    image = replaceBuffer(image, config.serverUrl, "SERVER_URL_abcdefghijklmnopqrstuvwxyz1234567890", fill);
    image = replaceBuffer(image, config.adapter, "NETWORK_ADAPTER", fill);

    if (config.adapter === 'wifi') {
        if (!config.wifiSsid || !config.wifiPassword)
            throw new Error("wifi ssid or wifi password is not provided");

        image = replaceBuffer(image, config.wifiSsid, "WIFI_SSID", fill);
        image = replaceBuffer(image, config.wifiPassword, "WIFI_PASSWORD", fill);
    }

    return image;
}

export function createFirmwareImage(image: Buffer): Buffer {
    const version = process.hrtime()[0] % 10000000; // FIXME
    const header = Buffer.alloc(16);
    header.writeUInt8(0x81, 0);
    header.writeUInt8(0xf1, 1);
    header.writeUInt8(0x5c, 2);
    header.writeUInt8(0x5e, 3);
    header.writeUInt32LE(image.length, 4);
    header.writeUInt32LE(version, 8);
    return Buffer.concat([header, image]);
}
