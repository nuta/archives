import * as crypto from 'crypto';
const storage = require('node-persist');
const hap = require('hap-nodejs');

interface AccessoryArgs {
    id: string;
    pin: string;
    name: string;
    manufacturer?: string;
    model?: string;
    serial?: string;
    category: string;
}

function generateRandomPort() {
    return 50000 + Math.floor(Math.random() * 10000);
}

export class Accessory {
    id: string;
    pin: string;
    deviceName: string;
    accessory: any;

    constructor({ id, pin, category, name, manufacturer = 'someone', model = 'proto', serial = 'abc' }: AccessoryArgs) {
        this.deviceName = name;
        this.pin = pin;
        this.id = id;

        storage.initSync({
            dir: '/tmp/homekit.storage' // FIXME
        });

        this.accessory = new hap.Accessory(this.deviceName,
            hap.uuid.generate('hap:accessories:' + this.deviceName));

        this.accessory
            .getService(hap.Service.AccessoryInformation)
            .setCharacteristic(hap.Characteristic.Manufacturer, manufacturer)
            .setCharacteristic(hap.Characteristic.Model, model)
            .setCharacteristic(hap.Characteristic.SerialNumber, serial);
    }

    start() {
        this.accessory.publish({
            pincode: this.pin,
            username: this.id,
            port: generateRandomPort()
        });
    }

    onIdentify(callback: (paired: boolean) => void) {
        this.accessory.on('identify', (paired: boolean, done: Function) => {
            callback(paired);
            done();
        })
    }
}
