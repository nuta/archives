export type DeviceState = 'ready' | 'running'
export type Configs = { [key: string]: string };

export interface PayloadMessages {
    appVersion?: string;
    osVersion?: string;
    appImageHMAC?: string;
    osImageHMAC?: string;
    hmac?: string;
    timestamp?: string;
    configs?: Configs;
    deviceId?: string;
    state?: string;
    log?: string;
    debugMode?: boolean;
}
