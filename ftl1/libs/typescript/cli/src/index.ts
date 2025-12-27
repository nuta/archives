import { COMMANDS } from "./commands";
import util from 'node:util';

function help() {
    console.info('Usage: ftl <subcommand>');
    console.info('');
    console.info('Subcommands:');
    console.info('');

    for (const [name, { help }] of Object.entries(COMMANDS)) {
        console.info(`  ${name.padEnd(8)} ${help}`);
    }

    console.info(`  ${'test'.padEnd(8)} Run tests (alias for "bun test")`);
    console.info(`  ${'help'.padEnd(8)} Show this help message`);
    console.info('');
}

export async function main(args: string[]): Promise<number> {
    const commandName = args[2];
    if (!commandName) {
        help();
        console.error('No command provided');
        return 1;
    }

    if (commandName === 'test') {
        // Rebuild the project before running tests.
        await main([...args.slice(0, 2), 'build']);

        const child = await Bun.spawn(['bun', 'test', ...args.slice(3)], { stdio: ['inherit', 'inherit', 'inherit'] });
        await child.exited;
        return child.exitCode ?? 11;
    }

    const cmd = COMMANDS[commandName];
    if (!cmd) {
        console.error(`Unknown command: ${commandName}`);
        return 1;
    }


    const parsed = util.parseArgs({ ...cmd.args, args: process.argv.slice(3) });
    try {
        await cmd.main(parsed);
    } catch (error) {
        console.error(`${commandName} failed: ${error}`);
        return 1;
    }

    return 0;
}
