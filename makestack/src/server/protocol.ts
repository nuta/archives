import { logger } from "../logger";

export interface Payload {
    deviceStatus?: {
        state: string,
        batteryLevel: number,
        ramFree: number,
    },
    version?: number,  /* FIXME: use bigint */
    firmwareRequest?: {
        version: number, /* FIXME: use bigint */
        offset: number,
    },
    firmwareData?: {
        type: "eof" | "raw" | "deflate",
        offset: number,
        data: Buffer,
    },
    corruptRateCheck?: {
        length: number,
    },
    ping?: {
        data: Buffer,
    },
    pong?: {
        data: Buffer,
    },
    log?: string,
}

function encodeLEB128(value: number): Buffer {
    let buf = Buffer.alloc(0);
    do {
        const msb = (value >= 0x80) ? 0x80 : 0;
        const byte = Buffer.from([msb | (value & 0x7f)]);
        buf = Buffer.concat([buf, byte]);
        value >>= 7;
    } while (value > 0);
    return buf;
}

// Returns a pair of [value, length].
function decodeLEB128(buf: Buffer): [number, number] {
    let value = 0;
    let shift = 0;
    for (let i = 0; i < buf.length && i < 4; i++) {
        value |= (buf[i] & 0x7f) << shift;
        if ((buf[i] & 0x80) == 0) {
            return [value, i + 1];
        }
        shift += 7;
    }

    throw new Error(`Invalid LEB128 format`);
}


function computeChecksum(data: Buffer): number {
    let checksum = 0;
    for (var i = 0; i < data.length; i++) {
        checksum += data[i];
        checksum &= 0xffff;
    }
    return checksum;
}

export function constructPayload(payload: Payload): Buffer {
    let payloadData = Buffer.alloc(0);
    if (payload.version) {
        const type = Buffer.from([0x01]);
        const data = Buffer.alloc(8);
        data.writeUInt32LE(payload.version, 0);
        payloadData = Buffer.concat([payloadData, type, encodeLEB128(data.length), data]);
    }

    if (payload.firmwareData) {
        const chunk = payload.firmwareData.data;
        const type = Buffer.from([0x02]);
        const data = Buffer.alloc(5 + chunk.length);
        const dataType = { raw: 0x01, deflate: 0x02, eof: 0xff }[payload.firmwareData.type];
        data.writeUInt8(dataType, 0);
        data.writeUInt32LE(payload.firmwareData.offset, 1);
        chunk.copy(data, 5);
        payloadData = Buffer.concat([payloadData, type, encodeLEB128(data.length), data]);
    }

    if (payload.corruptRateCheck) {
        const type = Buffer.from([0x03]);
        const data = Buffer.alloc(payload.corruptRateCheck.length);
        const patterns = [0x00, 0x5a, 0x0a, 0xff, 0xff, 0xa5, 0xee, 0xc0];
        for (let i = 0; i < data.length; i++) {
            data[i] = patterns[i % patterns.length];
        }
        payloadData = Buffer.concat([payloadData, type, encodeLEB128(data.length), data]);
    }

    if (payload.ping) {
        const type = Buffer.from([0x04]);
        const data = payload.ping.data;
        payloadData = Buffer.concat([payloadData, type, encodeLEB128(data.length), data]);
    }

    const checksum = Buffer.alloc(2);
    checksum.writeUInt16LE(computeChecksum(payloadData), 0);
    const payloadLen = Buffer.alloc(2);
    payloadLen.writeUInt16LE(payloadData.length, 0);
    return Buffer.concat([payloadLen, checksum, payloadData]);
}

export function parsePayload(buf: Buffer): Payload {
    const payloadData = buf.slice(4);
    const payloadLen = buf.readUInt16LE(0);
    if (payloadLen != payloadData.length) {
        throw new Error("invalid payload len");
    }

    const checksum = buf.readUInt16LE(2);
    const computed = computeChecksum(payloadData);
    if (checksum != computed) {
        throw new Error(`invalid payload checksum (computed=${computed}, expected=${checksum})`);
    }

    let firmwareRequest;
    let pong;
    let log;
    let deviceStatus;
    let offset = 4;
    while (offset + 2 < buf.length) {
        const type = buf[offset];
        const [length, lengthLength] = decodeLEB128(buf.slice(offset + 1));
        const dataOffset = offset + 1 + lengthLength;
        const data = buf.slice(dataOffset, dataOffset + length);

        switch (type) {
        case 0x05:
            pong = { data };
            break;
        case 0x06:
            log = data.toString("ascii");
            break;
        case 0x07:
            deviceStatus = {
                state: "", // TODO:
                batteryLevel: data.readUInt8(1),
                ramFree: data.readUInt32LE(4),
            };
            break;
        case 0xaa:
            if (data.length != 12) {
                logger.warn("malformed field (size of firmware_request is wrong)");
                break;
            }

            firmwareRequest = {
                version: data.readUInt32LE(0), // TODO: read 64bit
                offset: data.readUInt32LE(8),
            };
            break;
        }
''
        offset += 1 + lengthLength + length;
    }

    return { pong, firmwareRequest, log, deviceStatus };
}
