import * as crypto from "crypto";
import * as fs from "fs";
import * as os from "os";
import * as path from "path";

export function mkdirp(dir) {
    const dirs = path.resolve(dir).split("/");
    let dirpath = "/";

    for (const relpath of dirs) {
        dirpath = path.join(dirpath, relpath);

        if (!fs.existsSync(dirpath)) {
            fs.mkdirSync(dirpath);
        }
    }
}

export function generateRandomString(n) {
    const buf = Buffer.alloc((n / 2) + 1);
    crypto.randomFillSync(buf);
    return buf.toString("hex").substring(0, n);
}

export function find(basedir) {
    const files = [];
    const dirs = [basedir];

    if (fs.statSync(basedir).isFile()) {
        return [basedir];
    }

    do {
        const currentDir = dirs.pop();
        const ls = fs.readdirSync(currentDir);
        for (const relpath of ls) {
            const filepath = path.join(currentDir, relpath);
            if (fs.statSync(filepath).isDirectory()) {
                dirs.push(filepath);
            } else {
                files.push(path.relative(basedir, filepath));
            }
        }
    } while (dirs.length > 0);

    return files;
}

export function createFile(filepath, body) {
    mkdirp(path.dirname(filepath));
    fs.writeFileSync(filepath, body);
}

export function generateTempPath() {
    return path.join(os.tmpdir(), generateRandomString(32) + ".img");
}
