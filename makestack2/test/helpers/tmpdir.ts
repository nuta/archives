import * as os from "os";
import * as path from "path";
import * as fs from "fs-extra";

export class TmpDir {
    public cwd: string;
    public tmpdir: string;

    constructor() {
        this.cwd = process.cwd();
        this.tmpdir = path.resolve(os.tmpdir(), `tmp-${process.hrtime()[0]}`);
        fs.mkdirpSync(this.tmpdir);
        process.chdir(this.tmpdir);
    }

    public restore() {
        process.chdir(this.cwd);
        fs.removeSync(this.tmpdir);
    }
};

export function enter(): TmpDir {
    return new TmpDir();
}
