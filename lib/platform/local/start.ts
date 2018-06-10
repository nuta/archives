import * as fs from "fs";
import * as path from "path";
import * as express from "express";
import * as bodyParser from "body-parser";
import * as telemata from "../../telemata";
import { loadPlugins } from "../../plugins";
import { endpoints } from "../../server";
import { getFirmwareVersion } from "../../firmware";

console.log("==> starting local runtime")
const httpServer = express();

httpServer.use(function(req: any, res, next) {
    req.rawBody = Buffer.alloc(0);

    req.on('data', function(chunk: Buffer) {
        req.rawBody = Buffer.concat([req.rawBody, chunk]);
    });;

    req.on('end', function() {
        next();
    });
});

const firmwarePath = path.resolve("firmware.bin");
const firmwareImage = fs.readFileSync(firmwarePath);
const appVersion = getFirmwareVersion(firmwareImage);
console.log(`   version: ${appVersion}`);

const plugins = loadPlugins(['http_adapter'], {
    firmwarePath
});

for (const plugin of plugins) {
    if (plugin.server) {
        plugin.server(httpServer);
    }

    if (plugin.receivePayload) {
        plugin.receivePayload((payload: Buffer) => {
            console.log(payload);
            telemata.process(payload);

            return telemata.serialize({
                commands: {
                    "Hello": "World!",
                    "this": "is it!",
                },
                update: {
                    type: 'bulk',
                    version: appVersion
                }
            }, { includeDeviceId: false, deviceSecret: 'asd' });
        });
    }
}

const app = require(path.resolve(process.cwd(), "./server"));
for (const [name, callback] of Object.entries(endpoints)) {
    // TODO: support other methods
    httpServer.get(name, callback);
}

const host = "0.0.0.0";
const port = process.env.PORT || 7878;
httpServer.listen(port, host as any);
console.log(`started a http server at ${host}:${port}`);
