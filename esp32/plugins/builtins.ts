declare function __busywait(usec: number): void;
declare function __print(str: string): void;
declare function __setMode(pin: number, mode: 0 | 1): void;
declare function __digitalWrite(pin: number, value: boolean): void;
declare function __digitalRead(pin: number): boolean;
declare function __i2cRead(address: number, length: number): number[];
declare function __i2cWrite(address: number, data: number[]): void;

function print(...args: string[]) {
    __print(args.join(' '))
}

function error(...args: string[]) {
    __print('!' + args.join(' '))
}

function publish(event: string, value: string) {
    __print('@' + event + ' ' + value)
}

var Timer = {
    busywait(usec: number): void {
        __busywait(usec);
    }
}

class GPIO {
    private pin: number;

    constructor(options: { pin: number, mode: 'in' | 'out' }) {
        if (typeof options.pin !== 'number') {
            throw new Error('GPIO: pin must be number')
        }

        this.pin = options.pin
        this.setMode(options.mode)
    }

    setMode(mode: 'in' | 'out') {
        switch (mode) {
            case 'in':
                __setMode(this.pin, 0);
                break;
            case 'out':
                __setMode(this.pin, 1);
                break;
            default:
                throw new Error('GPIO: invalid mode');
        }

    }

    writeSync(value: boolean) {
        __digitalWrite(this.pin, value);
    }

    readSync(): boolean {
        return __digitalRead(this.pin);
    }
}


type Configs = { [key: string]: string }
type OnChangeCallback = (value: string) => void;
type OnCommandCallback = (value: string) => void;

var __configs: Configs = {};
var __onChangeCallbacks: { [key: string]: OnChangeCallback } = {};
var __onCommandCallbacks: { [key: string]: OnChangeCallback } = {};

var Config = {
    onCommand(key: string, callback: OnChangeCallback) {
        __onCommandCallbacks[key] = callback;
    },
    onChange(key: string, callback: OnChangeCallback) {
        if (__configs[key] !== undefined) {
            callback(__configs[key]);
        }

        __onCommandCallbacks[key] = callback;
    }
}

function __update_config(key: string, value: string) {
    if (__onChangeCallbacks[key] && __configs[key] !== value) {
        __configs[key] = value;
        __onChangeCallbacks[key](value);
    }
}

function __execute_command(key: string, value: string) {
    if (__onCommandCallbacks[key]) {
        const id = value.slice(0, value.indexOf(':'));
        const arg = value.slice(id.length + 1);
        const returnValue = __onCommandCallbacks[key](arg);
        print(`<${id} ${returnValue}`);
    }
}

// Node.js EventEmitter (https://nodejs.org/api/events.html)
type EventCallback = (...args: any[]) => void;
class EventEmitter {
    private callbacks: { [event: string]: EventCallback };
    constructor() {
        this.callbacks = {}
    }

    on(event: string, callback: EventCallback) {
        this.callbacks[event] = callback;
    }

    emit(event: string, ...args: any[]) {
        const callback = this.callbacks[event];
        if (callback) {
            callback(...args);
            return true;
        } else {
            return false;
        }
    }
}

// Node.js Buffer class (https://nodejs.org/api/buffer.html)
class Buffer {
    private data: number[];

    constructor(data: number[]) {
        this.data = data
    }

    static from(data: number[]) {
        return new Buffer(data)
    }

    slice(start?: number, end?: number) {
        return Buffer.from(this.data.slice(start, end))
    }

    readInt8(offset: number): number {
        return this.data[offset];
    }

    writeInt8(value: number, offset: number): void {
        this.data[offset] = value;
    }

    toArray() {
        return this.data;
    }
}

class I2C {
    private address: number;

    constructor(args: { address: number }) {
        this.address = args.address;
    }

    readSync(length: number): Buffer {
        return Buffer.from(__i2cRead(this.address, length));
    }

    writeSync(data: number[] | Buffer): void {
        if (data instanceof Buffer) {
            data = data.toArray();
        }

        __i2cWrite(this.address, data);
    }
}

interface Timer {
    initial: number;
    current: number;
    interval: boolean;
    callback: () => void;
}

var __loop_interval = 4294967295 // the maximum integer
const __timers: Timer[] = []

var setInterval = (callback: () => void, msec: number) => {
    __loop_interval = Math.min(__loop_interval, msec)

    __timers.push({
        interval: true,
        initial: msec,
        current: msec,
        callback
    })
}

var setTimeout = (callback: () => void, msec: number) => {
    __loop_interval = Math.min(__loop_interval, msec)

    __timers.push({
        interval: false,
        initial: msec,
        current: msec,
        callback
    })
}

// Periodically called by Engine.
var __loop = (delay: number) => {
    for (const timer of __timers) {
        if (timer.initial === 0) {
            continue;
        }

        timer.current -= delay;
        if (timer.current <= 0) {
            timer.callback();
            if (timer.interval) {
                timer.current = timer.initial;
            } else {
                // Mark the timer as deactivated.
                timer.initial = 0;
            }
        }
    }
}

var modules: { [name: string]: any } = {
    events: EventEmitter
}

var require: any = (moduleName: string): any => {
    return modules[moduleName]
}
