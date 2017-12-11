export type TDeviceState = 'ready' | 'running'
export type TConfigs = { [key: string]: string };

export interface IPayloadMessages {
    appVersion?: string;
    osVersion?: string;
    appImageHMAC?: string;
    osImageHMAC?: string;
    hmac?: string;
    timestamp?: string;
    configs?: TConfigs;
    deviceId?: string;
    state?: string;
    log?: string;
    debugMode?: boolean;
}
