export type TDeviceState = 'ready' | 'running'
export type TStores = { [key: string]: string };

export interface IPayloadMessages {
    appVersion?: string;
    osVersion?: string;
    appImageHMAC?: string;
    osImageHMAC?: string;
    hmac?: string;
    timestamp?: string;
    stores?: TStores;
    deviceId?: string;
    state?: string;
    log?: string;
    debugMode?: boolean;
}
