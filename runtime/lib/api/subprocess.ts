import { spawnSync } from "child_process";

interface SubProcessResult {
    stdout: string;
    stderr: string;
    status: number;
}

export class SubProcessAPI {
    public run(argv: string[]) {
        if (!Array.isArray(argv)) {
            throw new Error(`argv must be an array`);
        }

        const { stdout, stderr, status } = spawnSync(argv[0], argv.slice(1), { encoding: "utf-8" });
        return { stdout, stderr, status };
    }
}
