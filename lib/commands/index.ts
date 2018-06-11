import { Command as NewCommand } from "./new";
import { Command as DeployCommand } from "./deploy";
import { Command as DevCommand } from "./dev";

export const commands = [
    NewCommand,
    DeployCommand,
    DevCommand,
] as any[];
