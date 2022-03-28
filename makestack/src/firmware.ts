import * as fs from "fs-extra";
import * as path from "path";
import { Board, BuildOptions } from "./boards";
import { Transpiler } from "./transpiler";
import { render, execScriptHook } from "./helpers";

export interface Credential {
    version: number, /* FIXME: use bigint */
}

export function extractCredentials(path: string): Credential {
    const image = fs.readFileSync(path);
    const CRED_HEADER_START = "__MAKESTACK_CRED_START__";
    const start = image.indexOf(CRED_HEADER_START);
    if (start < -1) {
        throw new Error("failed to locate the credential");
    }

    const cred = image.slice(start + CRED_HEADER_START.length);
    const version = cred.readUInt32LE(0);
    return { version };
}

const APP_CXX_TEMPLATE = `\
#include <makestack/vm.h>
#include <makestack/logger.h>

{{ code }}
`

export function transpileApp(appDir: string): string {
    const appFile = path.join(appDir, "app.js");
    const appJs = fs.readFileSync(appFile, "utf-8");
    const transpiler = new Transpiler();
    const code = transpiler.transpile(appJs);
    return render(APP_CXX_TEMPLATE, { code });
}

export async function buildApp(board: Board, appDir: string, opts: BuildOptions) {
    execScriptHook(appDir, "build");
    await board.buildFirmware(appDir, transpileApp(appDir), opts);
}
