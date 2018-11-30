import * as express from "express";
import { Request, Response } from "express";
import * as request from "request-promise";
import { logger } from "../../logger";
import { AdapterCallback, Plugin } from "../../plugins";

function unhexlify(value: string): Buffer {
    let buf = Buffer.alloc(0);
    for (let i = 0; i < value.length; i += 2) {
        const v = Buffer.from([parseInt(value.substr(i, 2), 16)]);
        buf = Buffer.concat([buf, v]);
    }

    return buf;
}

function hexlify(value: Buffer): string {
    return value.toString("hex");
}

export default class SakuraioAdapter extends Plugin {
    public receivedCallback?: AdapterCallback;

    public async sendPayload(payload: Buffer): Promise<void> {
    }

    public receivePayload(callback: AdapterCallback) {
        this.receivedCallback = callback;
    }

    public async push(moduleId: string, payload: Buffer) {
        const BYTES_PER_CHANNEL = 8;

        const hexlified = hexlify(payload);
        const data = [];

        // "....AABBCCDD" => [..., "AABBCCDD"]
        for (let i = 0; i < hexlified.length; i += BYTES_PER_CHANNEL) {
            data.push(hexlified.slice(i, i + BYTES_PER_CHANNEL));
        }

        // [..., "AABBCCDD"] => [..., "AABBCCDD00000000"]
        data[data.length - 1] = data[data.length - 1].padEnd(8 * 2, "0");

        const channels: any[] = data.map((datum, index) => {
            return {
                channel: index,
                type: "b",
                value: datum,
            };
        });

        // Commit
        channels.push({ channel: 16, type: "i", value: 0x55aa });

        const body = {
            type: "channels",
            module: moduleId,
            payload: { channels },
        };

        try {
            await request({
                url: `https://api.sakura.io/incoming/v1/${process.env.SAKURAIO_WEBHOOK_TOKEN}`,
                method: "POST",
                body,
                json: true,
            });
        } catch (e) {
            logger.error(e);
        }
    }

    public server(httpServer: express.Express) {
        const telemataHandler = async (req: Request, res: Response) => {
            if (!this.receivedCallback) {
                throw new Error("received callback is not set");
            }

            const json: any = JSON.parse((req as any).rawBody.toString("utf-8"));

            if (json.type !== "channels") {
                res.status(200);
                return;
            }

            let i = 0;
            let payload = Buffer.alloc(0);
            const CHANNELS_MAX = 15;
            for (; i < CHANNELS_MAX; i++) {
                for (const { channel, type, value } of json.payload.channels) {
                    if (type === "b" && channel === i) {
                        payload = Buffer.concat([payload, unhexlify(value)]);
                        break;
                    }
                }
            }

            if (payload.length === 0) {
                logger.warn("sakura.io: too short payload, ignoring...");
                res.status(200);
                return;
            }

            const reply = await this.receivedCallback(payload);
            if (reply) {
                this.push(json.module, reply);
            }

            res.status(200);
        };

        const firmwareUrlHandler = (req: Request, res: Response) => {
            throw new Error("not yet implemented");
/*
            const urlBase = FIXME
            const firmwareUrl = `${urlBase}/makestack/sakuraio/download-firmware`;
            res.status(200).send(firmwareUrl);
*/
        };

        const firmwareDownloadHandler = (req: Request, res: Response) => {
            throw new Error("not yet implemented");
/*
            const boardType = "esp32"; // FIXME:
            if (!["esp32"].includes(boardType)) {
                logger.warn(`invalid board type: ${boardType}`);
                res.status(400);
                return;
            }

            res.status(200).sendFile(path.resolve(`./${boardType}.firmware`));
*/
        };

        // cloud
        httpServer.post("/sakuraio/telemata", telemataHandler);
        httpServer.get("/sakuraio/firmware", firmwareUrlHandler);
        httpServer.get("/sakuraio/download-firmware", firmwareDownloadHandler);

        // local
        httpServer.post("/makestack/sakuraio/telemata", telemataHandler);
        httpServer.get("/makestack/sakuraio/firmware", firmwareUrlHandler);
        httpServer.get("/makestack/sakuraio/download-firmware", firmwareDownloadHandler);
    }
}
