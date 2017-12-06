import { prepare } from "../prepare";

export async function main(args: any, opts: any) {
    await prepare(opts.appDir);
}
