import { CommandArgs } from ".";
import type { ParseArgsConfig } from "node:util";
import * as buildCommand from './build';
import { startQemu } from "../qemu";

export const name = 'run';
export const help = 'Run the emulator';
export const args: ParseArgsConfig = {
    options: {
        ...buildCommand.args.options,
    },
}

type Args = CommandArgs<typeof args>;

export async function main(args: Args): Promise<void> {
    await buildCommand.main(args);
    const proc = await startQemu({ arch: args.values.arch as string, stdio: 'inherit', debugMode: true, ports: [{ protocol: 'tcp', guestPort: 80, hostPort: 30080 }] });
    await proc.exited;
}
