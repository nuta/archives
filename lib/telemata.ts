import * as zlib from "zlib";
import { Device } from "./device";
import { createFirmwareImage } from "./firmware";
import { callbacks, HeartbeatCallback } from "./server";
import { logger } from "./logger";

const SMMS_VERSION = 1;
const SMMS_HMAC_MSG      = 0x01;
const SMMS_CACHE_MSG     = 0x02;
const SMMS_DEVICE_NAME_MSG = 0x03;
const SMMS_LOG_MSG       = 0x04;
const SMMS_COMMAND_MSG   = 0x05;
const SMMS_GET_MSG       = 0x06;
const SMMS_OBSERVE_MSG   = 0x07;
const SMMS_REPORT_MSG    = 0x08;
const SMMS_CONFIG_MSG    = 0x09;
const SMMS_UPDATE_MSG    = 0x0a;
const SMMS_OSUPDATE_MSG  = 0x0b;
const SMMS_CURRENT_VERSION_REPORT = 0x0001;

export type DeviceState = "ready" | "running";
export interface Reports {
    currentVersion?: number;
}
export interface Configs { [key: string]: string; }
export interface Commands { [key: string]: string; }

export interface PayloadMessages {
    reports?: Reports;
    configs?: Configs;
    commands?: Commands;
    update?: {
        type: "inline" | "bulk",
        data?: Buffer,
        version: number,
    };
    deviceId?: string;
    log?: string;
}

export function generateVariableLength(buf: Buffer) {
    let len = buf.length;
    let lenbuf = Buffer.alloc(0);

    while (len > 0) {
        const digit = len % 0x80;
        len = Math.floor(len / 0x80);
        buf = Buffer.from([((len > 0) ? 0x80 : 0) | digit]);
        lenbuf = Buffer.concat([lenbuf, buf]);
    }

    return ((lenbuf.length > 0) ? lenbuf : Buffer.from([0x00]));
}

export function parseVariableLength(buf: Buffer) {
    let length = 0;
    let i = 0;
    let base = 1;
    while (true) {
        if (i === buf.length) {
            throw new Error("invalid variable length");
        }

        const byte = buf[i];
        length += (byte & 0x7f) * base;

        if ((byte & 0x80) === 0) {
            return [length, i + 1];
        }

        i++;
        base *= 128;
    }
}

export function generateMessage(type: number, payload: any) {
    const buf = Buffer.from(payload);

    const lenbuf = generateVariableLength(buf);
    const msg = Buffer.alloc(1 + lenbuf.length + buf.length);
    msg.writeUInt8(type, 0);
    lenbuf.copy(msg, 1); // size
    buf.copy(msg, 1 + lenbuf.length);

    return msg;
}

export interface SerializeOptions {
    includeDeviceId: boolean;
    deviceSecret: string;
}

export function serialize({ deviceId, log, reports, configs, update, commands }: PayloadMessages) {
    let payload = Buffer.alloc(0);

    if (log) {
        const logMsg = generateMessage(SMMS_LOG_MSG, log);
        payload = Buffer.concat([payload, logMsg]);
    }

    if (reports) {
        if (reports.currentVersion) {
            const idBuffer = Buffer.alloc(2);
            idBuffer.writeUInt16BE(SMMS_CURRENT_VERSION_REPORT, 0);
            const valueBuffer = Buffer.alloc(4);
            valueBuffer.writeUInt32BE(reports.currentVersion, 0);
            const data = Buffer.from([idBuffer, valueBuffer]);
            const reportMsg = generateMessage(SMMS_REPORT_MSG, data);
            payload = Buffer.concat([payload, reportMsg]);
        }
    }

    if (configs) {
        for (const [key, value] of Object.entries(configs)) {
            const type = Buffer.from([0x01]); /* string */
            const configMsg = generateMessage(SMMS_CONFIG_MSG, Buffer.concat([
                type,
                generateVariableLength(Buffer.from(key)),
                Buffer.from(key),
                Buffer.from(value.toString()),
            ]));

            payload = Buffer.concat([payload, configMsg]);
        }
    }

    if (commands) {
        for (const [key, value] of Object.entries(commands)) {
            const type = Buffer.from([0x01]); /* string */
            const commandMsg = generateMessage(SMMS_COMMAND_MSG, Buffer.concat([
                type,
                generateVariableLength(Buffer.from(key)),
                Buffer.from(key),
                Buffer.from(value.toString()),
            ]));

            payload = Buffer.concat([payload, commandMsg]);
        }
    }

    if (update) {
        // Used by tests.
        const data = Buffer.alloc(5);
        data.writeUInt8(2, 0); // Download method
        data.writeUInt32BE(update.version, 1);
        const updateMsg = generateMessage(SMMS_UPDATE_MSG, data);
        payload = Buffer.concat([payload, updateMsg]);
    }

    let header = Buffer.alloc(1);
    header.writeUInt8(SMMS_VERSION << 4, 0);
    header = Buffer.concat([header, generateVariableLength(payload)]);

    return Buffer.concat([header, payload]);
}

export function deserialize(payload: Buffer) {
    const version = payload.readUInt8(0);
    if (version >> 4 !== SMMS_VERSION) {
        throw new Error("unsupported smms version");
    }

    const [totalLength, totalLengthLength] = parseVariableLength(payload.slice(1));
    const headerLength = 1 + totalLengthLength;
    const messages: any = { commands: {}, configs: {} };
    let offset = headerLength;
    while (offset < headerLength + totalLength) {
        const type = payload[offset];
        const [length, lengthLength] = parseVariableLength(payload.slice(offset + 1));
        const dataOffset = offset + 1 + lengthLength;
        const data = payload.slice(dataOffset, dataOffset + length);

        switch (type) {
            case SMMS_CONFIG_MSG: {
                const keyLengthOffset = dataOffset;
                const [keyLength, keyLengthLength] = parseVariableLength(payload.slice(keyLengthOffset));
                const keyOffset = keyLengthOffset + keyLengthLength;
                const valueOffset = keyOffset + keyLength;
                const valueLength = length - (valueOffset - keyLengthOffset);
                const key = payload.slice(keyOffset, keyOffset + keyLength);
                const value = payload.slice(valueOffset, valueOffset + valueLength);
                messages.configs[key.toString("utf-8")] = value.toString("utf-8");
                break;
            }
            case SMMS_COMMAND_MSG: {
                const keyLengthOffset = dataOffset;
                const [keyLength, keyLengthLength] = parseVariableLength(payload.slice(keyLengthOffset));
                const keyOffset = keyLengthOffset + keyLengthLength;
                const valueOffset = keyOffset + keyLength;
                const valueLength = length - (valueOffset - keyLengthOffset);
                const key = payload.slice(keyOffset, keyOffset + keyLength);
                const value = payload.slice(valueOffset, valueOffset + valueLength);
                messages.commands[key.toString("utf-8")] = value.toString("utf-8");
                break;
            }
            case SMMS_DEVICE_NAME_MSG:
                messages.update = data.toString("utf-8")
                break;
            case SMMS_LOG_MSG:
                messages.log = data.toString("utf-8")
                break;
            case SMMS_UPDATE_MSG:
                messages.update = {
                    version: data.readUInt32BE(1),
                };
                break;
            case SMMS_OSUPDATE_MSG:
                messages.osupdate = {
                    version: data.readUInt32BE(1),
                };
                break;
        }
        offset += 1 + lengthLength + length;
    }

    return messages;
}

export function parseLog(log: string): any {
    const events = [];
    const eventRegex = /^@([^ ]+) (.*)$/;
    for (const line in log.split("\n")) {
        const match = eventRegex.exec(line);
        if (match) {
            events.push({ name: match[0], value: match[1] });
        }
    }

    return { events };
}

export async function process(payload: Buffer): Promise<Device> {
    const { deviceName, log } = deserialize(payload);
    logger.info(`Heartbeat from ${deviceName}`);
    const device = await Device.getByName(deviceName);
    const { events } = parseLog(log || "");

    for (const callback of callbacks.heartbeat) {
        callback(device);
    }

    for (const { event, value } of events) {
        const callback = callbacks.event[event];
        if (!callback) {
            continue;
        }

        callback(device, event, value);
    }

    return device;
}
