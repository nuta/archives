import * as fs from "fs";
import * as zlib from "zlib";
import { parsePayload, constructPayload } from "./protocol";
import { bytesToReadableString } from "../helpers";
import { logger } from "../logger";
import { extractCredentials } from "../firmware";

export class ProtocolServer {
    private firmwareVersion!: number;
    private firmwareImage!: Buffer;
    private eventCallback: (name: string, value: any) => void;
    public verifiedPong: boolean = false;

    constructor(firmwarePath: string, eventCallback: (name: string, value: any) => void) {
        const firmwareVersion = extractCredentials(firmwarePath).version;
        const firmwareImage = fs.readFileSync(firmwarePath);
        this.firmwareVersion = firmwareVersion;
        this.firmwareImage = firmwareImage;
        this.eventCallback = eventCallback;
    }

    public buildHeartbeatPayload(): Buffer {
        return constructPayload({
            version: this.firmwareVersion,
            ping: {
                data: Buffer.from("HELO"),
            },
            corruptRateCheck: {
                length: 512,
            },
        });
    }

    public processPayload(rawPayload: Buffer):  Buffer | null {
        const payload = parsePayload(rawPayload);
        if (payload.deviceStatus) {
            const { ramFree } = payload.deviceStatus;
            console.log(`ram free: ${ramFree} bytes (${bytesToReadableString(ramFree)})`);
        }

        if (payload.log) {
            for (const line of payload.log.split("\n")) {
                const EVENT_REGEX = /^@(?<name>[^ ]+) (?<type>[bis]):(?<value>.*)$/;
                const m = line.match(EVENT_REGEX);
                if (m) {
                    const { name, type, value: valueStr } = m.groups!;
                    let value: any;
                    switch (type) {
                    case "b": value = (valueStr == "true"); break;
                    case "i": value = parseInt(valueStr); break;
                    case "s": value = valueStr; break;
                    default:
                        logger.warn(`unknown event type: \`${type}'`);
                        continue;
                    }

                    console.log(`event: name=${name}, value=${value}`);
                    this.eventCallback(name, value);
                } else {
                    console.log("device log:", line);
                }
            }
        }

        if (payload.pong) {
            this.verifiedPong = true;
        }

        if (payload.firmwareRequest) {
            if (payload.firmwareRequest.version != this.firmwareVersion) {
                logger.warn(
                    `invalid version: ${payload.firmwareRequest.version}`
                );
                return null;
            }

            const offset = payload.firmwareRequest.offset;
            const DATA_LEN = 8192;
            const data = this.firmwareImage.slice(offset, offset + DATA_LEN);
            let firmwareDataPayload;
            if (data.length > 0) {
                const compressed = zlib.deflateSync(data);
                if (compressed.length < data.length) {
                    firmwareDataPayload = constructPayload({
                        firmwareData: {
                            offset,
                            type: "deflate",
                            data: compressed,
                        },
                    });
                } else {
                    firmwareDataPayload = constructPayload({
                        firmwareData: { offset, type: "raw", data },
                    });
                }
            } else {
                firmwareDataPayload = constructPayload({
                    firmwareData: { offset, type: "eof", data },
                });
            }

            console.log(
                `Uploading len=${data.length}, offset=${offset} (${Math.floor(
                    (offset / this.firmwareImage.length) * 100
                )}%)`
            );

            return firmwareDataPayload;
        }

        return null;
    }
}
