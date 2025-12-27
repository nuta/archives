import * as build from './build';
import * as run from './run';
import * as dev from './dev';
import * as debug from './debug';
import * as deploy from './deploy';

import type { ParseArgsConfig, parseArgs } from 'node:util';

export type CommandArgs<T extends ParseArgsConfig> = ReturnType<typeof parseArgs<T>>;

export interface Command<T extends ParseArgsConfig> {
    name: string;
    help: string;
    main: (args: CommandArgs<T>) => Promise<void>;
    args: T;
}

export const COMMANDS: Record<string, Command<any>> = {
    build,
    run,
    dev,
    debug,
    deploy,
}
