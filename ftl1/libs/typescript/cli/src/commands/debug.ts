import type { ParseArgsConfig } from "node:util";
import { CommandArgs } from ".";
import { Socket } from "node:net";

export const name = 'debug';
export const help = 'Attach debugger to the emulator (LLDB)';
export const args: ParseArgsConfig = {
    options: {},
}

type Args = CommandArgs<typeof args>;


// Checks if the GDB server is listening on the default port (7778).
async function probeGdbPort(): Promise<boolean> {
    return new Promise((resolve, reject) => {
        const socket = new Socket();
        socket.connect(7778, '127.0.0.1');

        socket.on('error', (error) => {
            resolve(false);
        });
        socket.on('connect', () => {
            socket.end();
            resolve(true);
        });
    });
}

export async function main(args: Args): Promise<void> {
    if (!await probeGdbPort()) {
        console.warn('Warning: GDB debug port is not available (hint: did you start "bin/ftl run" first?)');
    }

    const script = [
        `target create build/ftl.elf`,
        `settings set stop-disassembly-display always`,
        `settings set stop-line-count-before 0`,
        `settings set stop-line-count-after 0`,
        `gdb-remote 127.0.0.1:7778`,
    ]

    const lldbArgs = []
    for (const line of script) {
        lldbArgs.push('--one-line', line);
    }

    const child = await Bun.spawn(
        ['rust-lldb', ...lldbArgs],
        {
            stdio: ['inherit', 'inherit', 'inherit'],
        }
    );

    await child.exited;
}
