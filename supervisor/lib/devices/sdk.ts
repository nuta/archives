import { DeviceBase } from "./device_base";

export class Device extends DeviceBase {
    public updateOS(image: Buffer) {
        throw new Error('updateOS is not available in sdk device')
    }
}
