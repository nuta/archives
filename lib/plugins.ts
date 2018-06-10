import * as express from "express";
import * as fs from "fs-extra";
import * as path from "path";

export type AdapterCallback = (payload: Buffer) => Buffer | undefined;
export interface PluginArgs {
    firmwarePath: string;
}

export class Plugin {
    protected firmwarePath: string;

    constructor(args: PluginArgs) {
        this.firmwarePath = args.firmwarePath;
    }

    server(express: express.Express): void {
    }

    async sendPayload(payload: Buffer): Promise<void> {
        return Promise.resolve();
    }

    receivePayload(callback: AdapterCallback): void {

    }
}

function getBuiltinPlugins(): string[] {
    const builtinPluginsDir = path.resolve(__dirname, '../plugins');

    return fs.readdirSync(builtinPluginsDir).map(name => {
        return path.join(builtinPluginsDir, name);
    })
}

export function loadPlugins(plugins: string[], args: PluginArgs): Plugin[] {
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
