import * as express from "express";
import * as fs from "fs-extra";
import * as path from "path";

export type AdapterCallback = (payload: Buffer) => Buffer | undefined;

export class Plugin {
    public server(express: express.Express): void {
    }

    public async sendPayload(payload: Buffer): Promise<void> {
        return Promise.resolve();
    }

    public receivePayload(callback: AdapterCallback): void {

    }
}

function getBuiltinPlugins(): string[] {
    const builtinPluginsDir = path.resolve(__dirname, "../../plugins");

    return fs.readdirSync(builtinPluginsDir).map((name) => {
        return path.join(builtinPluginsDir, name);
    });
}

function loadPluginPackageJson(dir: string): any {
    const packageJson = fs.readJsonSync(path.resolve(dir, "package.json"));
    const config = packageJson.makestack;
    config.name = packageJson.name;
    config.dir = dir;
    return config;
}

export function loadPlugins(required: string[]): { [name: string]: any } {
    const builtinPluginPaths = getBuiltinPlugins();
    const pluginPaths = [...builtinPluginPaths];
    const plugins: any[] = [];
    for (const pluginPath of pluginPaths) {
        if (!fs.statSync(pluginPath).isDirectory()) {
            continue;
        }

        const plugin = loadPluginPackageJson(pluginPath);
        const name = (builtinPluginPaths.includes(pluginPath)) ?
        plugin.name.replace("@makestack/", "") : plugin.name;

        if (!required.includes(name)) {
            continue;
        }

        plugins[plugin.name] = plugin;
    }

    if (required) {
        const missing = required.filter(pkg => pkg in plugins);
        if (missing.length > 0) {
            throw new Error(`missing plugins: ${missing.join(", ")}`);
        }
    }

    return plugins;
}

export function instantiatePlugins(plugins: string[]): Plugin[] {
    const modules = [];
    const builtinPlugins = getBuiltinPlugins().map(plugin => path.basename(plugin));
    for (const plugin of Object.values(loadPlugins(plugins))) {
        let ctor;
        const name = path.basename(plugin.name); // Remove `@makestack/' prefix.
        if (builtinPlugins.includes(name)) {
            /* Load from dist/plugins */
            ctor = require(path.resolve(__dirname, "../plugins/" + name)).default as any;
        } else {
            ctor = require(plugin.dir).default as any;
        }
        modules.push(new ctor());
    }
    return modules;
}
