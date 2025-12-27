import path from 'node:path';
import fs from 'node:fs/promises';
import type { ParseArgsConfig } from 'node:util';
import { CommandArgs } from '.';

export const name = 'build';
export const help = 'Build the project';
export const args: ParseArgsConfig = {
    options: {
        arch: {
            type: 'string',
            default: 'arm64',
        },
        release: {
            type: 'boolean',
            default: false,
        },
        timings: {
            type: 'boolean',
            default: false,
        },
    },
}

type Args = CommandArgs<typeof args>;

export async function main(args: Args): Promise<void> {
    const argv = [
        'cargo', 'build',
        '-Z', 'build-std=core,alloc',
        '-Z', 'build-std-features=compiler-builtins-mem',
        '--target', `kernel/src/arch/${args.values.arch}/kernel.json`,
        '--manifest-path', 'kernel/Cargo.toml',
    ];

    if (args.values.release) {
        argv.push('--release');
    }

    if (args.values.timings) {
        argv.push('--timings');
    }

    const proc = Bun.spawn(argv, {
        env: {
            ...process.env,
            CARGO_TERM_COLOR: 'always',
            CARGO_TERM_HYPERLINKS: 'false',
            CARGO_TERM_PROGRESS_WHEN: 'never',
            CARGO_FUTURE_INCOMPAT_REPORT_FREQUENCY: 'never',
            CARGO_TARGET_DIR: 'build',
        },
        stdio: [
            'inherit',
            'inherit',
            'inherit',
        ],
    });

    await proc.exited;
    if (proc.exitCode !== 0) {
        throw new Error(`build failed with ${proc.exitCode}`);
    }

    const binaryPath = path.resolve('build', 'kernel', args.values.release ? 'release' : 'debug', 'ftl_kernel');
    await fs.copyFile(binaryPath, 'build/ftl.elf');
}
