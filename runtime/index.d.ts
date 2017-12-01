interface Store {
  onCommand(key: string, callback: (newValue: string) => void): void;
  onChange(key: string, callback: (newValue: string) => void): void;
}

interface Logging {
  print(message: string): void;
  error(message: string): void;
}

interface Event {
  publish(event: string, data?: string | number): void;
}

interface Timer {
  loop(callback: () => void): void;
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

interface GPIO {
  INPUT: string;
  OUTPUT: string;
  new(args: { pin: number, mode: string });
  setMode(mode: string): void;
  write(value: boolean): void;
  read(): boolean;
  onInterrupt(mode: 'rising' | 'falling' | 'both', callback: () => void): void;
  onInterrupt(/* mode = 'rising' */ callback: () => void): void;
}

interface I2C {
  new(args: { address: number });
  read(length: number): Buffer;
  write(data: number[] | Buffer): void;
}

interface SPI {
  new(args: { slave: number, speed: number, mode: string });
  transfer(tx: number[] | Buffer): Buffer;
}

interface Serial {
  new(args: { path: string, baudrate: number });
  list(): string[];
  read(): Buffer;
  write(data: Buffer): void;
  onData(callback: (data: Buffer) => void): void;
  onNewLine(callback: (line: string) => void): void;
}

declare var Store: Store;
declare var Logging: Logging;
// declare var Event: Event;
declare var Timer: Timer;
declare var Subprocess: Subprocess;
declare var GPIO: GPIO;
declare var I2C: I2C;
declare var SPI: SPI;
declare var Serial: Serial;
