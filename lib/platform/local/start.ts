import * as bodyParser from "body-parser";
import * as express from "express";
import * as fs from "fs-extra";
import * as path from "path";
import { getFirmwareVersion } from "../../firmware";
import { instantiatePlugins } from "../../plugins";
import { endpoints } from "../../server";
import * as telemata from "../../telemata";
import { Logger } from "../../logger";

const logger = new Logger("server");
logger.debug("Staring the runtime");
const httpServer = express();

httpServer.use(express.static("public"));
httpServer.use(function(req: any, res, next) {
    req.rawBody = Buffer.alloc(0);

    req.on("data", function(chunk: Buffer) {
        req.rawBody = Buffer.concat([req.rawBody, chunk]);
    });

    req.on("end", function() {
        next();
    });
});

const firmwarePath = path.resolve("firmware.bin");
const firmwareImage = fs.readFileSync(firmwarePath);
const appVersion = getFirmwareVersion(firmwareImage);

logger.debug("Loading plugins");
const config = fs.readJsonSync("./package.json")["makestack"];
if (!config || !config["devPlugins"]) {
    throw new Error("Specify makestack.devPlugins in package.json");
}

const plugins = instantiatePlugins(config["devPlugins"], { firmwarePath });

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

const app = require(path.resolve(process.cwd(), "./server"));
for (const [name, callback] of Object.entries(endpoints)) {
    // TODO: support other methods
    httpServer.get(name, callback);
}

const host = "0.0.0.0";
const port = process.env.PORT || 7878;
httpServer.listen(port, host as any);
logger.debug(`Started a http server at http://${host}:${port}`);
logger.debug(`    version: ${appVersion}`);
