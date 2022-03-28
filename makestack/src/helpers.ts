import * as fs from "fs-extra";
import * as path from "path";
import * as tmp from "tmp";
import * as nunjucks from "nunjucks";
import { spawnSync, SpawnSyncOptions } from "child_process";
import { logger } from "./logger";

export class UserError extends Error {}

export function resolveRepoPath(relpath: string): string {
    return path.join(__dirname, "..", relpath);
}

export function bufferReplace(buf: Buffer, from: Buffer, to: Buffer): Buffer {
    let current = 0;
    for (;;) {
        const index = buf.indexOf(from, current);
        if (index < 0) {
            return buf;
        }

        const before = buf.slice(0, index);
        const after = buf.slice(index + from.length);

        // In case to.length < from.length.
        const newLen = index + to.length + after.length;

        buf = Buffer.concat([before, to, after], newLen);
        current = index + to.length;
    }
}

export function exec(argv: string[], options: SpawnSyncOptions = {}) {
    const exe = argv[0];
    const args = argv.slice(1);
    const r = spawnSync(exe, args, Object.assign({}, {
        stdio: "inherit",
        env: process.env,
        encoding: "utf-8",
    }, options));

    if (r.status !== 0) {
        throw new Error(`${exe} exited with ${r.status}`);
    }
}

export function bytesToReadableString(bytes: number): string {
    let base = bytes;
    let i = 0;
    const units = [" bytes", "KB", "MB", "GB"];
    while (base >= 1024 && i < units.length) {
        base /= 1024;
        i++;
    }

    return base.toFixed(2) + units[i];
}

export function render(template: string, ctx: { [name: string]: string }) {
    nunjucks.configure({ autoescape: false });
    return nunjucks.renderString(template, ctx);
}

export function createTmpDir(prefix: string): string {
    return tmp.dirSync({ prefix }).name;
}

export function execScriptHook(appDir: string, hook: string) {
    const scripts = fs.readJSONSync(path.join(appDir, "package.json")).scripts;
    if (scripts && scripts[hook]) {
        logger.progress(`Running ${hook} hook`);
        logger.command(scripts[hook]);
        exec(["sh", "-c", scripts[hook]], { cwd: appDir });
    }
}
