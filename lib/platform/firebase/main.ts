import * as express from "express";
import * as functions from "firebase-functions";
import * as fs from "fs";
import * as path from "path";
import { getFirmwareVersion } from "../../firmware";
import { instantiatePlugins } from "../../plugins";
import { endpoints } from "../../server";
import * as telemata from "../../telemata";

if (!process.env.APP_DIR) {
    throw new Error("BUG: APP_DIR is not set");
}

const httpServer = express();
httpServer.use(function(req: any, res, next) {
    req.rawBody = Buffer.alloc(0);

    req.on("data", function(chunk: Buffer) {
        req.rawBody = Buffer.concat([req.rawBody, chunk]);
    });

    req.on("end", function() {
        next();
    });
});

const firmwarePath = path.resolve(process.env.APP_DIR, "firmware.bin");
const firmwareImage = fs.readFileSync(firmwarePath);
const appVersion = getFirmwareVersion(firmwareImage);

const plugins = instantiatePlugins(["http_adapter"]);

for (const plugin of plugins) {
    if (plugin.server) {
        plugin.server(httpServer);
    }

    if (plugin.receivePayload) {
        plugin.receivePayload((payload: Buffer) => {
            const device = telemata.process(payload);
            return telemata.serialize({
                commands: device.data.commands,
                update: { type: "bulk", version: appVersion }
            });
        });
    }
}

const app = require(path.resolve(process.env.APP_DIR, "./server"));
for (const [name, callback] of Object.entries(endpoints)) {
    // TODO: support other methods
    httpServer.get(name, callback);
}

export const api = functions.https.onRequest(httpServer);
