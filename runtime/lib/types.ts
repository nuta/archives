export type GPIOPinMode = 'in' | 'out'
export type GPIOInterruptMode = 'rising' | 'falling' | 'both'

export interface GPIOInterface {
    setMode(mode: GPIOPinMode): void;
    writeSync(value: boolean): void;
    readSync(): boolean;
    onInterrupt(mode: GPIOInterruptMode, callback: () => void): void;
}

export interface GPIOConstructor {
    new(args: { pin: number, mode: GPIOPinMode }): GPIOInterface;
}

export type SPIMode = 'MODE0' | 'MDOE1' | 'MODE2';
export type SPIOrder = 'LSBFIRST' | 'MSBFIRST';
export interface SPIConstructor {
    new(args: {
        slave?: number,
        mode: SPIMode,
        speed?: number,
        order?: SPIOrder,
        bits?: number,
        ss?: number,
        path?: string
    }): SPIInterface;
}

export interface SPIInterface {
    transfer(tx: number[] | Buffer): Buffer;
}

export interface I2CConstructor {
    new(args: { address: number }): I2CInterface;
}

export interface I2CInterface {
    readSync(length: number): Buffer;
    writeSync(data: number[] | Buffer): void;
}
