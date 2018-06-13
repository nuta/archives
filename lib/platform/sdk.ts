import { DeployOptions } from "../types";

export abstract class PlatformSdk {
    abstract async deploy(appDir: string, opts: DeployOptions): Promise<void>;
    abstract viewLog(opts: any): void;
    abstract async command(deviceName: string, command: string, arg: string): Promise<any>;
}
