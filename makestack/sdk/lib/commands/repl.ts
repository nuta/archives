import * as readline from "readline";
import * as util from "util";
import { api } from "../api";

async function input(rl: any): Promise<string> {
    return new Promise((resolve, reject) => {
        rl.question(">>> ", resolve);
    }) as Promise<string>;
}

async function sleep(sec: number) {
    return new Promise((resolve, reject) => setTimeout(resolve, sec * 1000));
}

export async function main(args: any, opts: any, logger: any) {
    const rl = readline.createInterface({
        input: process.stdin,
        output: process.stdout,
    });

    while (true) {
        const line: string = await input(rl);
        const { id } = await api.invokeCommand(args.name, "__repl__", line);
        while (true) {
            const results = (await api.getCommandResults(args.name)).results;
            if (id in results) {
                const result = JSON.parse(results[id].split("__repl__ ")[1]);
                console.log(util.inspect(result, { colors: true }));
                break;
            }
            await sleep(1);
        }
    }
}
