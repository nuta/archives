import * as caporal from "caporal";
import * as path from "path";
import { generatePdf } from "./pdf";
const { version } = require(path.resolve(__dirname, "../package.json"));

export interface CliArgs {
    file: string;
}

export interface CliOpts {
    output: string;
}

async function doRun(args: CliArgs, opts: CliOpts) {
    await generatePdf(args.file, opts.output ? opts.output : "output.pdf");
}

export async function run(args?: string[]) {
    caporal
        .version(version)
        .argument("file", "The markdown file.")
        .option("-o <output>", "The output file.")
        .action(doRun as any);

    const argv = args ? [process.argv0, "supershow", ...args] : process.argv;
    await caporal.parse(argv);
}
