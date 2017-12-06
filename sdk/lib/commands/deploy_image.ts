import * as fs from "fs";
import { api } from "../api";

export async function main(args: any, opts: any, logger: any) {
    await api.deploy(opts.app, fs.readFileSync(args.image));
}
