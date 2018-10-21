import { DeployOptions } from "../types";

export abstract class PlatformSdk {
    public abstract async deploy(appDir: string, opts: DeployOptions): Promise<void>;
    public abstract viewLog(opts: any): void;
    public abstract async command(deviceName: string, command: string, arg: string): Promise<any>;
}
