import * as fs from "fs";
import * as path from "path";
import { mkdirp } from "./helpers";

const CONFIG_DIR = process.env.CONFIG_DIR || `${process.env.HOME}/.makestack`;

function getConfigPath(name) {
    return `${CONFIG_DIR}/${name}.json`;
}

function load(name) {
    return JSON.parse(fs.readFileSync(getConfigPath(name), { encoding: "utf-8" }));
}

function save(name, data) {
    mkdirp(path.dirname(getConfigPath(name)));
    fs.writeFileSync(getConfigPath(name), JSON.stringify(data));
}

export function loadCredentials() {
    return load("credentials");
}

export function saveCredentials(data) {
    return save("credentials", data);
}

export function loadMocks() {
    return load("mocks");
}

export function updateMocks(data) {
    const mocks = (fs.existsSync(getConfigPath("mocks"))) ? loadMocks() : {};
    save("mocks", Object.assign(mocks, data));
}

export function saveMocks(data) {
    return save("mocks", data);
}
