// The entry point of the app (server side) invoked by the dev server. The
// current directory is the app directory.
import * as path from "path";
import * as express from "express";
import * as proxy from "http-proxy-middleware";
import { DevServerRequest } from "./dev_server";
const chalk = require("chalk");
const { initServer } = require(path.join(__dirname, "../server"));
chalk.enabled = true;
chalk.level = 2;

function main() {
    // These environment variables are should be passed by DevServer.
    const host = process.env.HOST!;
    const port = parseInt(process.env.PORT!);
    const devServerPort = parseInt(process.env.DEV_SERVER_PORT!);

    // Handle messages from the dev server.
    process.on("message", (req: DevServerRequest) => {
        switch (req.type) {
        case "event":
            const callback = (global as any).__eventEndpoints[req.name];
            if (callback) {
                callback(req.value);
            }
            break;
        }
    });

    const server = express();
    server.use((req, res, next) => {
        console.log(chalk.magenta.bold(`[HTTP] ${req.method} ${req.path}`));
        next();
    });

    // Forward requests to the dev server.
    server.use(proxy("/makestack", {
        target: `http://localhost:${devServerPort}`,
        changeOrigin: true
    }));

    // Serve static files in the public.
    server.use(express.static("public"));

    const appJsPath = path.resolve(process.cwd(), "app.js");
    initServer(appJsPath, server);
    server.listen(port, host);
}

main();
