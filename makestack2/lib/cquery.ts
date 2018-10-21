import * as path from "path";
import { parse } from "shell-quote";

export interface CompileCommand {
    directory: string;
    file: string;
    arguments: string[];
}

export function generateCompileCommandsJson(stdout: string, additionalCppflags: string[]): string {
    const CXX_PATTERNS = /\-g?c\+\+\ .+\-c\ /;

    const commands: CompileCommand[] = [];
    for (const line of stdout.split("\n")) {
        if (CXX_PATTERNS.exec(line)) {
            const args = parse(line);
            const file = args.filter((f) => f.endsWith(".cpp") || f.endsWith(".cc") || f.endsWith(".cxx"))[0];
            commands.push({
                directory: path.resolve(__dirname, "../../esp32"),
                file,
                arguments: args.concat(additionalCppflags),
            });
        }
    }

    return JSON.stringify(commands, null, 4);
}
