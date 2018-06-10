import { Platform, DeviceData } from "../../types";

export class LocalPlatform extends Platform {
    private devices: { [deviceName: string]: DeviceData };

    constructor() {
        super();
        this.devices = {};
    }

    async getDeviceData(deviceName: string): Promise<DeviceData> {
        return this.devices[deviceName] || {};
    }

    async setDeviceData(deviceName: string, data: DeviceData) {
        this.devices[deviceName] = data;
    }
}
