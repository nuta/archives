import * as buildCommand from './build';
import type { CommandArgs } from './index';
import type { ParseArgsConfig } from "node:util";
import path from 'node:path';
import fs from 'node:fs/promises';
import { startQemu } from '../qemu';

export const name = 'dev';
export const help = 'Rapid development mode';
export const args: ParseArgsConfig = {
    options: {
        ...buildCommand.args.options,
    },
}

const SOURCE_EXTENSIONS = new Set([
    '.rs',
    '.toml',
    '.ts',
    '.js',
    '.json',
    '.html',
]);

type Args = CommandArgs<typeof args>;

export async function main(args: Args): Promise<void> {
    let qemu: ReturnType<typeof Bun.spawn> | null = null;

    process.on('exit', () => {
        if (qemu) {
            qemu.kill('SIGTERM');
        }
    });

    const rebuild = async (filename?: string) => {
        if (qemu) {
            qemu.kill('SIGTERM');
        }

        // Clear the screen.
        if (process.stdout.isTTY) {
            console.log('\x1b[2J\x1b[H');
        }

        if (filename) {
            console.log(`Changed: ${filename}`);
        }

        try {
            await buildCommand.main(args);
            if (qemu) {
                await qemu.exited;
            }
            qemu = await startQemu({ arch: args.values.arch as string, stdio: 'inherit', debugMode: true, ports: [{ protocol: 'tcp', guestPort: 80, hostPort: 30080 }] });
        } catch (error) {
            console.error(`failed to run: ${error}`);
        }
    };

    const watchDir = path.resolve(import.meta.dir, '..', '..', '..', '..', '..');
    console.log(`Watching for changes in ${watchDir}...`);
    await rebuild();
    const watcher = fs.watch(watchDir, { recursive: true });
    for await (const { eventType, filename } of watcher) {
        if (!filename || filename.startsWith('build/') || filename.startsWith('target/')) {
            continue;
        }

        if (!SOURCE_EXTENSIONS.has(path.extname(filename))) {
            continue;
        }

        // TODO: debounce?
        await rebuild(filename);
    }
}
