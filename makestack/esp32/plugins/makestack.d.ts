declare function print(...args: string[]): void;
declare function error(...args: string[]): void;
declare function publish(event: string, value: string): void;

declare class GPIO {
    constructor(options: { pin: number, mode: 'in' | 'out' });
    setMode(mode: 'in' | 'out'): void;
    writeSync(value: boolean): void;
    readSync(): boolean;
}

declare var Timer: {
    busywait(usec: number): void;
}

declare type OnChangeCallback = (value: string) => void;
declare type OnCommandCallback = (value: string) => void;
declare var Config: {
    onCommand(key: string, callback: OnChangeCallback): void;
    onChange(key: string, callback: OnChangeCallback): void;
}

declare type EventCallback = (...args: any[]) => void;
declare class EventEmitter {
    constructor();
    on(event: string, callback: EventCallback): void;
    emit(event: string, ...args: any[]): boolean;
}

declare class Buffer {
    constructor(data: number[]);
    static from(data: number[]): void;
    slice(start?: number, end?: number): void;
    readInt8(offset: number): number;
    writeInt8(value: number, offset: number): void;
    toArray(): number[];
}

declare class I2C {
    constructor(args: { address: number });
    readSync: (length: number) => Buffer;
    writeSync: (data: number[] | Buffer) => void;
}

declare var modules: { [name: string]: any } ;
declare function require(moduleName: string): any;
