import { DeviceData } from "./types";
import { getRuntimeInstance } from "./platform/index";
import { PlatformRuntime } from "./platform/runtime";

function isDeepEqual(obj1: object, obj2: object) {
    return JSON.stringify(obj1) === JSON.stringify(obj2);
}

export type BoardType = "esp32";
const DEVICE_TYPE_ESP32 = 1;

export function getBoardNameByDeviceType(id: number): BoardType {
    switch (id) {
        case DEVICE_TYPE_ESP32:
            return "esp32";
        default:
            throw new Error(`Unknown device type: ${id}`);
    }
}

export interface DeviceState {
    version: number;
    board: BoardType;
    ramFree?: number;
    battery?: number;
};

export class Device {
    public name: string;
    public data?: DeviceData;
    public state?: DeviceState;
    private platform: PlatformRuntime;
    private initialData?: DeviceData;
    private commands?: { [name: string]: string };

    constructor(name: string) {
        this.name = name;
        this.platform = getRuntimeInstance();
    }

    public async load() {
        this.initialData = await this.platform.getDeviceData(this.name);
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
            await this.platform.setDeviceData(this.name, this.data);
        }
    }
}
