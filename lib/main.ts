import * as caporal from "caporal";
import * as fs from "fs";
import * as path from "path";
import * as http from "http";
import * as Event from "events";
import * as express from "express";
import * as WebSocket from "ws";
import { Request, Response } from "express";
import { logger } from "./logger";
const { version } = require(path.resolve(__dirname, "../package.json"));

export interface CliArgs {
    file: string;
}

function doRun(args: CliArgs) {
    // Poll for file changes.
    const fileEvent = new Event();
    fs.watch(args.file, (ev) => {
        logger.info("Detected a change, updating...");
        const newText = fs.readFileSync(args.file, { encoding: "utf-8" });
        if (newText.length > 0) {
            fileEvent.emit("change", newText);
        }
    })

    // Launch a local server.
    const app = express();
    const server = http.createServer(app);
    app.use(express.static(path.resolve(__dirname, "ui")));
    app.use(express.static(path.resolve(path.dirname(args.file))));

    // Launch a WebSocket server for auto update.
    const wsServer = new WebSocket.Server({ server });
    wsServer.on("connection", (ws: WebSocket) => {
        fileEvent.on("change", (newText: string) => {
            ws.send(JSON.stringify({
                event: "changed",
                text: newText
            }))
        })
    })

    server.listen(2020, () => {
        logger.progress("Started a local server at http://localhost:2020");
    });
}

export function run(args?: string[]) {
    caporal.version(version);
    caporal.argument("file", "The markdown file.");
    caporal.action(doRun as any);

    const argv = args ? [process.argv0, "makestack", ...args] : process.argv;
    caporal.parse(argv);
}
