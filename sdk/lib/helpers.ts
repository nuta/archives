import * as crypto from "crypto";
import * as fs from "fs";
import * as os from "os";
import * as path from "path";
import { spawnSync, SpawnSyncOptions } from "child_process";
import { FatalError } from "./types";

export function removeFiles(filepath: string): void {
    if (!fs.existsSync(filepath)) {
        return;
    }

    const remaining = [filepath];
    while (true) {
        const target = remaining.pop();
        if (!target) {
            break
        }

        if (fs.statSync(target).isDirectory) {
            const innerFiles = fs.readdirSync(target).map((name) => path.join(target, name));
            if (innerFiles.length === 0) {
                fs.rmdirSync(target);
            } else {
                remaining.concat(innerFiles);
            }
        } else {
            // A normal file.
            fs.unlinkSync(target);
        }
    }
}

export function mkdirp(dir: string) {
    const dirs = path.resolve(dir).split("/");
    let dirpath = "/";

    for (const relpath of dirs) {
        dirpath = path.join(dirpath, relpath);

        if (!fs.existsSync(dirpath)) {
            fs.mkdirSync(dirpath);
        }
    }
}

export function generateRandomString(n: number) {
    const buf = Buffer.alloc((n / 2) + 1);
    crypto.randomFillSync(buf);
    return buf.toString("hex").substring(0, n);
}

export function find(basedir: string) {
    const files = [];
    const dirs = [basedir];

    if (fs.statSync(basedir).isFile()) {
        return [basedir];
    }

    while (true) {
        const currentDir = dirs.pop();
        if (!currentDir) {
            break;
        }

        const ls = fs.readdirSync(currentDir);
        for (const relpath of ls) {
            const filepath = path.join(currentDir, relpath);
            if (fs.statSync(filepath).isDirectory()) {
                dirs.push(filepath);
            } else {
                files.push(path.join(basedir, path.relative(basedir, filepath)));
            }
        }
    }

    return files;
}

export function createFile(filepath: string, body: Buffer | string) {
    mkdirp(path.dirname(filepath));
    fs.writeFileSync(filepath, body);
}

export function generateTempPath(prefix = '', suffix = ''): string {
    return path.join(os.tmpdir(), 'makestack-' + prefix + generateRandomString(32) + suffix);
}

export function getenv(name: string): string {
    const value = process.env[name]
    if (!value) {
        throw new Error(`process.env[${name}] is not defined`)
    }

    return value;
}

export function run(argv: string[], options: SpawnSyncOptions = {}) {
    const exe = argv[0]
    const args = argv.slice(1)
    const { status } = spawnSync(exe, args, Object.assign({}, options, {
        stdio: "inherit",
        env: process.env,
    }));

    if (status !== 0) {
        throw new FatalError(`docker build exited with ${status}`)
    }
}

export function readTextFile(path: string) {
    return fs.readFileSync(path, { encoding: 'utf-8' });
}
