export type DeviceData = { [key: string]: any };

export abstract class Platform {
    abstract async getDeviceData(deviceName: string): Promise<DeviceData>;
    abstract async setDeviceData(deviceName: string, data: DeviceData): Promise<void>;
}

export abstract class Board {
    abstract async build(appDir: string): Promise<void>;
}

export interface DeployOptions {
    firebaseProject?: string;
}
