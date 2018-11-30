import { GPIOInterface, GPIOPinMode, GPIOInterruptMode } from "../../types";
const fs = require("fs");

export class LinuxGPIOAPI implements GPIOInterface {
    public pin: number;

    constructor(args: { pin: number, mode: GPIOPinMode }) {
        if (typeof args.pin !== "number") {
            throw new Error("`this.pin' must be a number");
        }

        this.pin = args.pin;
        this.setMode(args.mode);
    }

    public setMode(mode: GPIOPinMode) {
        if (mode !== 'in' && mode !== 'out') {
            throw new Error(`invalid pin mode \`${mode}'`);
        }

        if (fs.existsSync(`/sys/class/gpio/gpio${this.pin}`)) {
            fs.writeFileSync(`/sys/class/gpio/unexport`, `${this.pin}`);
        }

        fs.writeFileSync(`/sys/class/gpio/export`, `${this.pin}`);

        // Try changing the direction until the ownership of
        // /sys/class/gpio/.../direction is updated by udev.
        let err;
        let retryCount = 30000;
        while (retryCount > 0) {
            try {
                fs.writeFileSync(`/sys/class/gpio/gpio${this.pin}/direction`,
                (mode === "in") ? "in" : "out");
            } catch (e) {
                err = e
                retryCount--;
                continue;
            }

            // Successfully changed the direction.
            break;
        }

        if (retryCount === 0) {
            throw new Error(`Failed to change the GPIO direction ${err}`)
        }
    }

    public writeSync(value: boolean) {
        fs.writeFileSync(`/sys/class/gpio/gpio${this.pin}/value`, value ? "1" : "0");
    }

    public readSync() {
        return fs.readFileSync(`/sys/class/gpio/gpio${this.pin}/value`, "utf-8") === "1\n";
    }

    public onInterrupt(mode: GPIOInterruptMode, callback: () => void) {
        this.setMode('in');
        fs.writeFileSync(`/sys/class/gpio/gpio${this.pin}/edge`, mode);
        fs.watch(`/sys/class/gpio/gpio${this.pin}/value`, () => {
            callback();
        });
    }

    public onChange(callback: () => void) {
        this.setMode('in');
        fs.writeFileSync(`/sys/class/gpio/gpio${this.pin}/edge`, "both");
        fs.watch(`/sys/class/gpio/gpio${this.pin}/value`, () => {
            callback();
        });
    }
}
