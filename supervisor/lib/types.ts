export type DeviceState = 'ready' | 'running';
export type Reports = {
    currentVersion?: number
};
export type Configs = { [key: string]: string };
export type Commands = { [key: string]: string };

export interface PayloadMessages {
    reports?: Reports;
    configs?: Configs;
    commands?: Commands;
    update?: {
        version: number
    };
    deviceId?: string;
    log?: string;
}
