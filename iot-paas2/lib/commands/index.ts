import { Command as CommandCommand } from "./command";
import { Command as DeployCommand } from "./deploy";
import { Command as DevCommand } from "./dev";
import { Command as InstallCommand } from "./install";
import { Command as LogCommand } from "./log";
import { Command as NewCommand } from "./new";

export const commands = [
    NewCommand,
    DeployCommand,
    DevCommand,
    InstallCommand,
    LogCommand,
    CommandCommand,
] as any[];
