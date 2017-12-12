import { buildPlugin } from "../plugin_builder";

export function main(args: any, opts: any, logger: any) {
    buildPlugin(opts.pluginDir, opts.outfile);
}
