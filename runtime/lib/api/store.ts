import { sendToSupervisor } from '../helpers';

type Stores = { [key: string]: string }
type onChangeCallback = (value: string) => void;
type onCommandCallback = (value: string) => void;

export class StoreAPI {
    public stores: Stores;
    public onChangeCallbacks: { [key: string]: onChangeCallback[] };
    public onCommandCallbacks: { [key: string]: onCommandCallback };

    constructor() {
        this.stores = {};
        this.onChangeCallbacks = {};
        this.onCommandCallbacks = {};
    }

    public onCommand(key: string, callback: (value: string) => void) {
        this.onCommandCallbacks[key] = callback;
    }

    public onChange(key: string, callback: (value: string) => void) {
        if (this.stores[key] !== undefined) {
            callback(this.stores[key]);
        }

        if (key in this.onChangeCallbacks) {
            this.onChangeCallbacks[key].push(callback);
        } else {
            this.onChangeCallbacks[key] = [callback];
        }
    }

    public async update(newStores: Stores) {
        for (const key in newStores) {
            if (key.startsWith(">")) {
                // Command
                const [commandId, commandKey] = key.substring(1).split(" ");
                if (this.onCommandCallbacks[commandKey]) {
                    const returnValue = await this.onCommandCallbacks[commandKey](newStores[key]);
                    sendToSupervisor("log", { body: `<${commandId} ${returnValue}` });
                }
            } else {
                // Store
                const oldValue = this.stores[key];
                const newValue = newStores[key];
                this.stores[key] = newValue;

                if (this.onChangeCallbacks[key] && oldValue !== newValue) {
                    for (const callback of this.onChangeCallbacks[key]) {
                        callback(newValue);
                    }
                }
            }
        }
    }
}
