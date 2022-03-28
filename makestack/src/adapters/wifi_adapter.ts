import * as express from "express";
import { logger } from "../logger";

export class WiFiAdapter {
    public constructor(server: express.Express, onRecv: (payload: Buffer) => Buffer | null) {
        server.use((req: any, res: express.Response, next: Function) => {
            req.rawBody = Buffer.alloc(0);

            req.on("data", function(chunk: Buffer) {
                req.rawBody = Buffer.concat([req.rawBody, chunk]);
            });

            req.on("end", function() {
                next();
            });
        });

        server.post("/makestack/protocol", (req, res) => {
            const reply = onRecv((req as any).rawBody);
            res.status(200);
            res.type("application/octet-stream");
            res.send(reply);
        });
    }
}
