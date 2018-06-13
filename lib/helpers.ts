import * as path from "path";
import * as fs from "fs-extra";
import { spawnSync, SpawnSyncOptions } from "child_process";
import { logger } from "./logger";

export function guessSerialFilePath(): string | null {
    const patterns = [
        /cu\.usbserial-[0-9]+/,
        /ttyUSB[0-9]/
    ];

    const candidates = fs.readdirSync("/dev")
        .filter(file =>
            patterns.some(pat => pat.exec(file) !== null)
        );

    if (candidates.length === 1) {
        return path.join("/dev", candidates[0]);
    } else {
        return null;
    }
}


export function exec(argv: string[], options: SpawnSyncOptions = {}) {

    const exe = argv[0]
    const args = argv.slice(1)
    const r = spawnSync(exe, args, Object.assign({}, {
        stdio: "inherit",
        env: process.env,
        encoding: "utf-8"
    }, options));

    if (r.status !== 0) {
        throw new Error(`${exe} exited with ${r.status}`)
    }
}

export function execWithPipe(argv: string[], options: SpawnSyncOptions = {}):
    { stdout: string, stderr: string }
{

    const exe = argv[0]
    const args = argv.slice(1)
    const r = spawnSync(exe, args, Object.assign({}, {
        stdio: "pipe",
        env: process.env,
        encoding: "utf-8"
    }, options));

    if (r.status !== 0) {
        throw new Error(`${exe} exited with ${r.status}`)
    }

    return {
        stdout: r.stdout.toString('utf-8'),
        stderr: r.stderr.toString('utf-8')
    }
}


export function downloadRepo(appDir: string): string {
    const repoDir = path.join(appDir, "build/repo");
    if (!fs.existsSync(path.join(appDir, "build/repo/.git"))) {
        logger.progress("Downloading the repository");
        fs.mkdirpSync(path.dirname(repoDir));
        exec(["git", "clone", "https://github.com/seiyanuta/makestack", "repo"], {
            cwd: path.join(appDir, "build")
        });
    }

    return repoDir;
}


// Don't pass user-provided variables to this function! It could be a
// seriaous vulnerability!
export function ensurePipInstalled(packages: string[]) {
    for (const name in packages) {
        if (spawnSync("python", ["-c", `import ${name}`]).status != 0) {
            throw new Error(`Install pip package "${name}"`);
        }
    }
}
