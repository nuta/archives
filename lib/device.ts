import { instantiatePlatform } from "./platform";
import { DeviceData } from "./types";

const platform = instantiatePlatform();

function isDeepEqual(obj1: object, obj2: object) {
    return JSON.stringify(obj1) === JSON.stringify(obj2);
}

export type BoardType = "esp32";

export class Device {
    public name: string;
    public data?: DeviceData;
    public board: BoardType;
    private initialData?: DeviceData;
    private commands?: { [name: string]: string };

    constructor(name: string) {
        this.name = name;
        this.board = "esp32"; // FIXME:
    }

    public async load() {
        this.initialData = await platform.getDeviceData(this.name);
        this.data = Object.assign({}, this.initialData);
        this.commands = this.data.commands || {};
    }

    public static async getByName(name: string): Promise<Device> {
        if (!name) {
            throw new Error("Invalid device name.");
        }

        const device = new Device(name);
        await device.load();
        return device;
    }

    public command(name: string, arg: string) {
        if (!this.commands) {
            throw new Error("Perform device.load() first!");
        }

        this.commands[name] = arg;
    }

    public dequeuePendingCommands(): { [name: string]: string } {
        if (!this.commands) {
            throw new Error("Perform device.load() first!");
        }

        const commands = this.commands;
        this.commands = {};
        return commands;
    }

    public async save() {
        if (!this.data || !this.initialData) {
            throw new Error("Perform device.load() first!");
        }

        this.data.commands = this.commands;

        if (!isDeepEqual(this.initialData, this.data)) {
            await platform.setDeviceData(this.name, this.data);
        }
    }
}
