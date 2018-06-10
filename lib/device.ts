import { platform } from "./platform";
import { DeviceData } from "./types";

function isDeepEqual(obj1: object, obj2: object) {
    return JSON.stringify(obj1) === JSON.stringify(obj2);
}

export class Device {
    public name: string;
    public data: DeviceData;
    private initialData: DeviceData;

    constructor(name: string) {
        this.name = name;
        this.initialData = platform.getDeviceData(name);
        this.data = Object.assign({}, this.initialData);
    }

    public saveIfChanged() {
        if (!isDeepEqual(this.initialData, this.data)) {
            platform.setDeviceData(this.name, this.data);
        }
    }
}
