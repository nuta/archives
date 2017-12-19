const { Service, Characteristic } = require(`../../deps/${process.arch}/hap-nodejs`);
import { Accessory } from "../accessory";

export class Light extends Accessory {
    name: string;

    constructor({ id, pin, name, manufacturer, model, serial }:
        { id: string, pin: string, name: string, manufacturer?: string, model?: string, serial?: string }
    ) {
        super({ id, pin, name, manufacturer, model, serial, category: 'light' });
        this.name = name;

        this.accessory.addService(Service.Lightbulb, this.name);
    }

    onOnGet(callback: () => boolean) {
        this.accessory
            .getService(Service.Lightbulb, this.name)
            .getCharacteristic(Characteristic.On)
            .on('get', (done: Function) => {
                done(null, callback());
            })
    }

    onOnSet(callback: (value: boolean) => void) {
        this.accessory
            .getService(Service.Lightbulb, this.name)
            .getCharacteristic(Characteristic.On)
            .on('set', (value: boolean, done: Function) => {
                callback(value);
                done();
            })
    }

    updateOnValue(value: boolean) {
        this.accessory
            .getService(Service.Lightbulb, this.name)
            .getCharacteristic(Characteristic.On)
            .updateValue(value)
    }

    onBrightnessGet(callback: () => any) {
        this.accessory
            .getService(Service.Lightbulb, this.name)
            .getCharacteristic(Characteristic.Brightness)
            .on('get', (done: Function) => {
                done(null, callback());
            })
    }

    onBrightnessSet(callback: (value: any) => void) {
        this.accessory
            .getService(Service.Lightbulb, this.name)
            .getCharacteristic(Characteristic.Brightness)
            .on('set', (value: any, done: Function) => {
                callback(value);
                done();
            })
    }

    updateBrightnessValue(value: any) {
        this.accessory
            .getService(Service.Lightbulb, this.name)
            .getCharacteristic(Characteristic.Saturation)
            .updateValue(value)
    }

    onSaturationGet(callback: () => any) {
        this.accessory
            .getService(Service.Lightbulb, this.name)
            .getCharacteristic(Characteristic.Saturation)
            .on('get', (done: Function) => {
                done(null, callback());
            })
    }

    onSaturationSet(callback: (value: any) => void) {
        this.accessory
            .getService(Service.Lightbulb, this.name)
            .getCharacteristic(Characteristic.Saturation)
            .on('set', (value: any, done: Function) => {
                callback(value);
                done();
            })
    }

    updateSaturationValue(value: any) {
        this.accessory
            .getService(Service.Lightbulb, this.name)
            .getCharacteristic(Characteristic.Saturation)
            .updateValue(value)
    }
};