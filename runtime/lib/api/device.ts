import { getDeviceType } from "../helpers";

export class DeviceAPI {
    public getDeviceType(): string {
        return getDeviceType();
    }
}
