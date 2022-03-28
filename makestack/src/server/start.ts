// The entry point of the app (server side) invoked by the dev server.
import * as fs from "fs";
import * as path from "path";
import * as express from "express";

export function initServer(appJsPath: string, server: express.Express) {
    let appJs = fs.readFileSync(appJsPath, "utf-8");

    // XXX: Send me a pull request to improve this part :)
    const packagePath = path.resolve(__dirname, "../..");
    appJs = appJs.replace('require("makestack")', `require("${packagePath}")`);
    eval(appJs);

    const endpoints: any = (global as any).__httpEndpoints;
    for (const [method, callbacks] of Object.entries(endpoints)) {
        for (const { name, callback } of (callbacks as any[])) {
            ((server as any)[method] as any)(name, callback);
        }
    }
}
