import { DeviceBase } from "./device_base";

export class Device extends DeviceBase {
    public updateOS(image: Buffer) {
        console.log('updateOS:', image)
    }
}
