import * as fs from "fs";
import * as path from "path";
import { mkdirp } from "./helpers";

const CONFIG_DIR = process.env.CONFIG_DIR || `${process.env.HOME}/.makestack`;

function getConfigPath(name: string) {
    return `${CONFIG_DIR}/${name}.json`;
}

function load(name: string): any | null {
    if (!fs.existsSync(getConfigPath(name))) {
        return null
    }

    return JSON.parse(fs.readFileSync(getConfigPath(name), { encoding: "utf-8" }));
}

function save(name: string, data: any) {
    mkdirp(path.dirname(getConfigPath(name)));
    fs.writeFileSync(getConfigPath(name), JSON.stringify(data));
}

export function loadCredentials() {
    return load("credentials");
}

export function saveCredentials(data: any) {
    return save("credentials", data);
}

export function loadDeviceConfig() {
    return load("device");
}

export function saveDeviceConfig(data: any) {
    return save("device", data);
}
