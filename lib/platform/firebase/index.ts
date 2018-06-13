import { deploy } from "./deploy";
import { viewLog } from "./log";
import { FirebasePlatform } from "./runtime";

async function command(deviceName: string, command: string, arg: string) {
    // TODO
}

export { deploy, viewLog, command };
export const Platform = FirebasePlatform;
