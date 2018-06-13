import * as bodyParser from "body-parser";
import * as express from "express";
import * as fs from "fs-extra";
import * as path from "path";
import { getFirmwareVersion } from "../../firmware";
import { Logger } from "../../logger";
import { instantiatePlugins } from "../../plugins";
import { endpoints } from "../../server";
import * as telemata from "../../telemata";

const logger = new Logger("server");
logger.debug("Staring the runtime");
const httpServer = express();

// Enable basic authentication to requests except /makestack/*.
if (process.env.BASIC_AUTH) {
    const basicAuthCredential = process.env.BASIC_AUTH.split(":", 2);
    const requestsFromDevices = [
        "/makestack/telemata",
        "/makestack/firmware",
    ];

    httpServer.use((req, res, next) => {
        if (requestsFromDevices.includes(req.path)) {
            // Currently no authentication mechanism are implementd.
            next();
        }

        const header = req.headers.authorization;
        if (!header) {
            res.status(401).header("WWW-Authenticate", 'Basic realm=""').end();
            return;
        }

        const base64Cred = header.split("Basic ")[1];
        if (!base64Cred) {
            res.status(401).header("WWW-Authenticate", 'Basic realm=""').end();
            return;
        }

        const cred = Buffer.from(base64Cred, "base64").toString();
        const [username, password] = cred.split(":", 2);
        if (username !== basicAuthCredential[0] || password !== basicAuthCredential[1]) {
            res.status(401).header("WWW-Authenticate", 'Basic realm=""').end();
            return;
        }

        next();
    });
}

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

import * as net from "net";
import { instantiatePlatform } from "..";
import { Device } from "../../device";
interface LocalRequest {
    type: string;
    command?: {
        name: string;
        device: string;
        arg: string;
    };

}
async function handleLocalRequest(req: LocalRequest) {
    switch (req.type) {
        case "command":
            if (!req.command) {
                return { result: "error" };
            }

            const device = await Device.getByName(req.command.device);
            device.command(req.command.name, req.command.arg);
            device.save();
            return { result: "success" };
        default:
            return { result: "error" };
    }
}

const sock = net.createServer();
sock.on("connection", (client) => {
    const str = "";
    client.on("data", async (data) => {
        let req;
        try {
            req = JSON.parse(data.toString("utf-8"));
        } catch (e) {
            logger.error(e);
            logger.error(e.stack);
            client.write(JSON.stringify({ result: "error" }));
        }
        client.write(JSON.stringify(await handleLocalRequest(req)));
    });
});

fs.removeSync("dev.sock");
sock.listen("dev.sock");

logger.debug("Loading plugins");
const config = fs.readJsonSync("./package.json").makestack;
if (!config || !config.devPlugins) {
    throw new Error("Specify makestack.devPlugins in package.json");
}

const plugins = instantiatePlugins(config.devPlugins);

for (const plugin of plugins) {
    if (plugin.server) {
        plugin.server(httpServer);
    }

    if (plugin.receivePayload) {
        plugin.receivePayload(async (payload: Buffer) => {
            const device = await telemata.process(payload);
            const firmwarePath = path.resolve(`firmware.${device.board}.bin`);
            const firmwareImage = fs.readFileSync(firmwarePath);
            const appVersion = getFirmwareVersion(firmwareImage);

            const reply = telemata.serialize({
                commands: device.dequeuePendingCommands(),
                update: { type: "bulk", version: appVersion },
            });

            device.save();
            return reply;
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
