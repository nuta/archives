import { NewCommand } from "./new_command";
import { DevCommand } from "./dev_command";
import { FlashCommand } from "./flash_command";
import { SerialCommand } from "./serial_command";
import { BuildCommand } from "./build_command";
import { DeployCommand } from "./deploy_command";
import { LogCommand } from "./log_command";

export const commands = [
    NewCommand,
    BuildCommand,
    DevCommand,
    DeployCommand,
    LogCommand,
    FlashCommand,
    SerialCommand,
] as any[];
