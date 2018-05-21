import { sendToSupervisor } from '../helpers';

export type Commands = { [key: string]: string }
export type Configs = { [key: string]: string }
export type onChangeCallback = (value: string) => void;
export type onCommandCallback = (value: string) => void;

export class ConfigAPI {
    public configs: Configs;
    public onChangeCallbacks: { [key: string]: onChangeCallback[] };
    public onCommandCallbacks: { [key: string]: onCommandCallback };

    constructor() {
        this.configs = {};
        this.onChangeCallbacks = {};
        this.onCommandCallbacks = {};
    }

    public onCommand(key: string, callback: (value: string) => void) {
        this.onCommandCallbacks[key] = callback;
    }

    public onChange(key: string, callback: (value: string) => void) {
        if (this.configs[key] !== undefined) {
            callback(this.configs[key]);
        }

        if (key in this.onChangeCallbacks) {
            this.onChangeCallbacks[key].push(callback);
        } else {
            this.onChangeCallbacks[key] = [callback];
        }
    }

    public async executeCommands(commands: Commands) {
         for (const [ key, value ] of Object.entries(commands)) {
            const id = value.slice(0, value.indexOf(':'));
            const arg = value.slice(id.length + 1);
            const returnValue = await this.onCommandCallbacks[key](arg);
            sendToSupervisor("log", { body: `<${id} ${returnValue}` });
        }
    }

    public async updateConfigs(newConfigs: Configs) {
        for (const key of Object.keys(newConfigs)) {
            const oldValue = this.configs[key];
            const newValue = newConfigs[key];
            this.configs[key] = newValue;

            if (this.onChangeCallbacks[key] && oldValue !== newValue) {
                for (const callback of this.onChangeCallbacks[key]) {
                    callback(newValue);
                }
            }
        }
    }
}
