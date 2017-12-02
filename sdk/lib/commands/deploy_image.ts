import * as fs from "fs";
import { api } from "../api";

export async function main(args, opts, logger) {
  await api.deploy(opts.app, fs.readFileSync(args.image));
}
