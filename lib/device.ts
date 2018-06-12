import { platform } from "./platform";
import { DeviceData } from "./types";

function isDeepEqual(obj1: object, obj2: object) {
    return JSON.stringify(obj1) === JSON.stringify(obj2);
}

export type BoardType = "esp32";

export class Device {
    public name: string;
    public data: DeviceData;
    public board: BoardType;
    private initialData: DeviceData;
    private commands: { [name: string]: string };

    constructor(name: string) {
        this.name = name;
        this.board = "esp32"; // FIXME:
        this.initialData = platform.getDeviceData(name);
        this.data = Object.assign({}, this.initialData);
        this.commands = this.data.commands || {};
    }

    private getByName(name: string) {
        return new Device(name);
    }

    public command(name: string, arg: string) {
        this.commands[name] = arg;
    }

    public saveIfChanged() {
        this.data.commands = this.commands;

        if (!isDeepEqual(this.initialData, this.data)) {
            platform.setDeviceData(this.name, this.data);
        }
    }
}
