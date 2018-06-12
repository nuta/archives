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

    if (value.length + 1 /* in case `fill' is a NULL chracter */ > id.length) {
        throw new Error(`too long value for ${id}: \`${value}'`);
    }

    const paddedValue = Buffer.alloc(needle.length, fill);
    const valueBuf = Buffer.from(value);
    valueBuf.copy(paddedValue);
    paddedValue.copy(buf, index);
    return buf;
}

export function getReplaceMeId(name: string, length: number): Buffer {
    const prefix = "__REPLACE_ME__" + name + "_";
    if (length - prefix.length < 0) {
        throw new Error(`BUG: too long replace me: ${name}`);
    }

    return Buffer.from(prefix + Buffer.alloc(length - prefix.length, "x"));
}

// struct {
//     char DEVICE_NAME[64];
//     char SERVER_URL[256];
//     char NETWORK_ADAPTER[32];
//     char WIFI_SSID[64];
//     char WIFI_PASSWORD[64];
// };
export const CREDENTIALS_DATA_TEMPLATE = Buffer.concat([
    getReplaceMeId("DEVICE_NAME", 64),
    getReplaceMeId("SERVER_URL", 256),
    getReplaceMeId("NETWORK_ADAPTER", 32),
    getReplaceMeId("WIFI_SSID", 64),
    getReplaceMeId("WIFI_PASSWORD", 64),
])

export function embedCredentials(image: Buffer, config: InstallConfig): Buffer {
    const fill = 0x00;
    image = replaceBuffer(image, config.deviceName, "DEVICE_NAME_abcdefghijklmnopqrstuvwxyz1234567890", fill);
    image = replaceBuffer(image, config.serverUrl, "SERVER_URL_abcdefghijklmnopqrstuvwxyz1234567890", fill);
    image = replaceBuffer(image, config.adapter, "NETWORK_ADAPTER", fill);

    if (config.adapter === 'wifi') {
        if (!config.wifiSsid || !config.wifiPassword)
            throw new Error("wifi ssid or wifi password is not provided");

        image = replaceBuffer(image, config.wifiSsid, "WIFI_SSID_abcdefghijklmnopqrstuvwxyz1234567890", fill);
        image = replaceBuffer(image, config.wifiPassword, "WIFI_PASSWORD_abcdefghijklmnopqrstuvwxyz1234567890", fill);
    }

    return image;
}

export function createFirmwareImage(version: number, image: Buffer): Buffer {
    const header = Buffer.alloc(16);
    header.writeUInt8(0x81, 0);
    header.writeUInt8(0xf1, 1);
    header.writeUInt8(0x5c, 2);
    header.writeUInt8(0x5e, 3);
    header.writeUInt32LE(image.length, 4);
    header.writeUInt32LE(version, 8);
    return Buffer.concat([header, image]);
}
