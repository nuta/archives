import * as caporal from "caporal";
import * as path from "path";
import { generatePdf } from "./pdf";
import { generatePptx } from "./pptx";
const { version } = require(path.resolve(__dirname, "../package.json"));

export interface CliArgs {
    file: string;
}

export interface CliOpts {
    output: string;
}

async function doRun(args: CliArgs, opts: CliOpts) {
    const output = opts.output ? opts.output : "output.pdf";
    switch (path.extname(output)) {
        case ".pdf":
            await generatePdf(args.file, output);
            break;
        case ".pptx":
            await generatePptx(args.file, output);
            break;
        default:
            throw new Error("Unsupported output file type: " + path.extname(output));
    }
}

export async function run(args?: string[]) {
    caporal
        .version(version)
        .argument("file", "The markdown file.")
        .option("-o,--output <output>", "The output file.")
        .action(doRun as any);

    const argv = args ? [process.argv0, "supershow", ...args] : process.argv;
    await caporal.parse(argv);
}
