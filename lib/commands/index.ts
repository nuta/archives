import { Command as NewCommand } from "./new";
import { Command as DeployCommand } from "./deploy";
import { Command as DevCommand } from "./dev";
import { Command as InstallCommand } from "./install";
import { Command as LogCommand } from "./log";

export const commands = [
    NewCommand,
    DeployCommand,
    DevCommand,
    InstallCommand,
    LogCommand,
] as any[];
