import { buildPlugin } from "../plugin_builder";

export function main(args: any, opts: any, logger: any) {
    buildPlugin(args.path, args.dest);
}
