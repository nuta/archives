import * as esp32 from "./esp32";

export interface BuildOptions {
    adapter: string,
    heartbeatInterval: number,
    wifiSsid?: string,
    wifiPassword?: string,
    serverUrl?: string,
};

export interface Board {
    flashFirmware: (appDir: string, appCxx: string, devicePath: string, opts: BuildOptions) => Promise<void>;
    buildFirmware: (appDir: string, appCxx: string, opts: BuildOptions) => Promise<void>;
    getFirmwarePath: () => string;
}

export class BuildError extends Error {}

export { esp32 as Board };
