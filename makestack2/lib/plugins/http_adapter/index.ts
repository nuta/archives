import * as express from "express";
import { Request, Response } from "express";
import * as path from "path";
import { logger } from "../../logger";
import { AdapterCallback, Plugin } from "../../plugins";

export default class HttpAdapter extends Plugin {
    public receivedCallback?: AdapterCallback;

    public async sendPayload(payload: Buffer): Promise<void> {
    }

    public receivePayload(callback: AdapterCallback) {
        this.receivedCallback = callback;
    }

    public server(httpServer: express.Express) {
        const telemataHandler = async (req: Request, res: Response) => {
            if (!this.receivedCallback) {
                throw new Error("received callback is not set");
            }

            const reply = await this.receivedCallback((req as any).rawBody);
            res.send(reply);
        };

        const firmwareHandler = (req: Request, res: Response) => {
            const boardType = req.query.board;
            if (!["esp32"].includes(boardType)) {
                logger.warn(`invalid board type: ${boardType}`);
                res.status(400);
                return;
            }

            res.sendFile(path.resolve(`./${boardType}.firmware`));
        };

        // cloud
        httpServer.post("/telemata", telemataHandler);
        httpServer.get("/firmware", firmwareHandler);

        // local
        httpServer.post("/makestack/telemata", telemataHandler);
        httpServer.get("/makestack/firmware", firmwareHandler);
    }
}
