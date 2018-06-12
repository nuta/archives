import * as fs from "fs-extra";
import * as express from "express";
import { Request, Response } from "express";
import { Plugin, AdapterCallback } from "../../lib/plugins";
import { parseVariableLength } from "../../lib/telemata";
import { createFirmwareImage } from "../../lib/firmware";

export default class HttpAdapter extends Plugin {
    receivedCallback?: AdapterCallback;

    async sendPayload(payload: Buffer): Promise<void> {
    }

    receivePayload(callback: AdapterCallback) {
        this.receivedCallback = callback;
    }

    server(httpServer: express.Express) {
        const telemataHandler = (req: Request, res: Response) => {
            if (!this.receivedCallback) {
                throw new Error("received callback is not set");
            }

            console.log((req as any).rawBody);
            const reply = this.receivedCallback((req as any).rawBody);
            console.log('replying: ', reply);
            res.send(reply);
        };

        const firmwareHandler = (req: Request, res: Response) => {
            const boardType = req.query.board;
            if (!/\A[a-z0-9]+\z/.exec(boardType)) {
                res.status(400);
                return;
            }

            res.send(`firmware.${boardType}.bin`);
        };

        // cloud
        httpServer.post("/telemata", telemataHandler);
        httpServer.get("/firmware", firmwareHandler);

        // local
        httpServer.post("/makestack/telemata", telemataHandler);
        httpServer.get("/makestack/firmware", firmwareHandler);
    }
}
