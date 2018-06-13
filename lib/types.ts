export interface DeviceData { [key: string]: any; }

export interface InstallConfig {
    wifiSsid?: string;
    wifiPassword?: string;
    serverUrl: string;
    deviceName: string;
    serial?: string;
    adapter: "wifi";
}

export abstract class Board {
    public abstract async build(repoDir: string, appDir: string): Promise<void>;
    public abstract async install(repoDir: string, appDir: string, config: InstallConfig): Promise<void>;
}

export interface DeployOptions {
    firebaseProject?: string;
}
