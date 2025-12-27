import type { ParseArgsConfig } from 'node:util';
import { CommandArgs } from '.';
import * as buildCommand from './build';
import { $ } from 'bun';

export const name = 'deploy';
export const help = 'Deploy to a remote machine';
export const args: ParseArgsConfig = {
    allowPositionals: true,
    options: {
        ...buildCommand.args.options,
        logs: {
            type: 'boolean',
            default: false,
        },
    },
};

function progress(message: string) {
    console.log(`\x1b[1;94m==>\x1b[0m \x1b[1m${message}\x1b[0m`);
}

type Args = CommandArgs<typeof args>;

export async function main(args: Args): Promise<void> {
    const sshHost = args.positionals[0];
    if (!sshHost) {
        throw new Error('host is required');
    }

    progress('Building FTL locally');
    await buildCommand.main(args);

    progress(`Preparing /opt/ftl on ${sshHost}`);
    await $`ssh ${sshHost} 'sudo mkdir -p /opt/ftl && sudo chown -R $USER:$USER /opt/ftl'`;

    progress('Uploading files');
    await $`rsync -avz build/ftl.elf deploy/setup.sh deploy/boot.sh deploy/ftl.service ${sshHost}:/opt/ftl/`;

    progress('Installing "ftl" systemd service');
    await $`ssh ${sshHost} /bin/sh /opt/ftl/setup.sh`;

    progress('Deployed FTL - serving HTTP on 0.0.0.0:30080');
    if (args.values.logs) {
        progress('Streaming logs ...');
        await $`ssh ${sshHost} "sudo journalctl -u ftl -f"`;
    }
}
