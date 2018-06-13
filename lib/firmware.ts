import { InstallConfig } from "./types";

export function getFirmwareVersion(firmware: Buffer): number {
    return firmware.readUInt32LE(8);
}


function replaceBuffer(buf: Buffer, value: string, needle: Buffer, fill: number): Buffer {
    const index = buf.indexOf(needle);
    if (index === -1) {
        throw new Error(`replaceBuffer: failed to replace ${needle.toString("utf-8")}`);
    }

    if (value.length + 1 /* in case `fill' is a NULL chracter */ > needle.length) {
        throw new Error(`too long value for ${needle}: \`${value}'`);
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
const REPLACE_ME_DEVICE_NAME = getReplaceMeId("DEVICE_NAME", 64);
const REPLACE_ME_SERVER_URL = getReplaceMeId("SERVER_URL", 256);
const REPLACE_ME_NETWORK_ADAPTER = getReplaceMeId("NETWORK_ADAPTER", 32);
const REPLACE_ME_WIFI_SSID = getReplaceMeId("WIFI_SSID", 64);
const REPLACE_ME_WIFI_PASSWORD = getReplaceMeId("WIFI_PASSWORD", 64);
export const CREDENTIALS_DATA_TEMPLATE = Buffer.concat([
    REPLACE_ME_DEVICE_NAME,
    REPLACE_ME_SERVER_URL,
    REPLACE_ME_NETWORK_ADAPTER,
    REPLACE_ME_WIFI_SSID,
    REPLACE_ME_WIFI_PASSWORD,
])

export function embedCredentials(image: Buffer, config: InstallConfig): Buffer {
    const fill = 0x00;
    image = replaceBuffer(image, config.deviceName, REPLACE_ME_DEVICE_NAME, fill);
    image = replaceBuffer(image, config.serverUrl, REPLACE_ME_SERVER_URL, fill);
    image = replaceBuffer(image, config.adapter, REPLACE_ME_NETWORK_ADAPTER, fill);

    if (config.adapter === 'wifi') {
        if (!config.wifiSsid || !config.wifiPassword)
            throw new Error("wifi ssid or wifi password is not provided");

        image = replaceBuffer(image, config.wifiSsid, REPLACE_ME_WIFI_SSID, fill);
        image = replaceBuffer(image, config.wifiPassword, REPLACE_ME_WIFI_PASSWORD, fill);
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
