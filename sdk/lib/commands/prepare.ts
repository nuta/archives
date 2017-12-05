import { prepare } from "../prepare";

export async function main(args, opts) {
    await prepare(opts.appDir);
}
