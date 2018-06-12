import * as express from "express";
import * as fs from "fs-extra";
import * as path from "path";
import { PackageConfig } from "./types";

export type AdapterCallback = (payload: Buffer) => Buffer | undefined;

export class Plugin {
    protected firmwarePath: string;

    constructor(args: PackageConfig) {
        this.firmwarePath = args.firmwarePath;
    }

    public server(express: express.Express): void {
    }

    public async sendPayload(payload: Buffer): Promise<void> {
        return Promise.resolve();
    }

    public receivePayload(callback: AdapterCallback): void {

    }
}

function getBuiltinPlugins(): string[] {
    const builtinPluginsDir = path.resolve(__dirname, "../plugins");

    return fs.readdirSync(builtinPluginsDir).map((name) => {
        return path.join(builtinPluginsDir, name);
    });
}

export function loadPlugins(plugins: string[], args: PackageConfig): Plugin[] {
    const pluginPaths = getBuiltinPlugins();
    const modules = [];
    for (const pluginPath of pluginPaths) {
        if (!fs.statSync(pluginPath).isDirectory()) {
            continue;
        }

        if (!plugins.includes(path.basename(pluginPath))) {
            continue;
        }

        const ctor = require(pluginPath).default as any;
        modules.push(new ctor(args));
    }

    return modules;
}
