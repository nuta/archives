/// <reference types="node" />

interface Config {
  onCommand(key: string, callback: (newValue: string) => void): void;
  onChange(key: string, callback: (newValue: string) => void): void;
}

interface Timer {
  loop(callback: () => void): void;
  delay(duration: number, callback: () => void): void;
  interval(interval: number, callback: () => void): void;
  sleep(duration: number): Promise<void>;
  busywait(usec: number): void;
}

interface SubProcessResult {
  stdout: string;
  stderr: string;
  status: number;
}

interface Subprocess {
  run(argv: string[]): SubProcessResult;
}

type GPIOPinMode = 'in' | 'out'
type GPIOInterruptMode = 'rising' | 'falling' | 'both'
interface GPIO {
  new(args: { pin: number, mode: GPIOPinMode }): GPIO;
  setMode(mode: GPIOPinMode): void;
  write(value: boolean): void;
  read(): boolean;
  onInterrupt(mode: GPIOInterruptMode, callback: () => void): void;
  onInterrupt(/* mode = 'rising' */ callback: () => void): void;
}

interface I2C {
  new(args: { address: number }): I2C;
  read(length: number): Buffer;
  write(data: number[] | Buffer): void;
}

type SPIMode = 'MODE0' | 'MDOE1' | 'MODE2';
type SPIOrder = 'LSBFIRST' | 'MSBFIRST';
interface SPI {
  new(args: { slave: number, speed: number, mode: SPIMode, order: SPIOrder, bits: number, ss: number, path: string }): SPI;
  transfer(tx: number[] | Buffer): Buffer;
}

interface Serial {
  new(args: { path: string, baudrate: number }): Serial;
  list(): string[];
  read(): Buffer;
  write(data: Buffer): void;
  onData(callback: (data: Buffer) => void): void;
  onNewLine(callback: (line: string) => void): void;
}

declare function println(message: string): void;
declare function error(message: string): void;
declare function publish(event: string, data?: string | number): void;
declare var Config: Config;
declare var Timer: Timer;
declare var Subprocess: Subprocess;
declare var GPIO: GPIO;
declare var I2C: I2C;
declare var SPI: SPI;
declare var Serial: Serial;
