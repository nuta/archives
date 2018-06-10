import { DeviceData, Platform } from "../../types";

export class LocalPlatform extends Platform {
    private devices: { [deviceName: string]: DeviceData };

    constructor() {
        super();
        this.devices = {};
    }

    public async getDeviceData(deviceName: string): Promise<DeviceData> {
        return this.devices[deviceName] || {};
    }

    public async setDeviceData(deviceName: string, data: DeviceData) {
        this.devices[deviceName] = data;
    }
}
