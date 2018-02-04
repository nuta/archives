export abstract class Serial {
    abstract async open(args: { path: string, baudRate: number }): Promise<void>;
    abstract close(): void;
    abstract async read(length: number): Promise<Uint8Array>;
    abstract async write(data: Uint8Array): Promise<void>;
    abstract async flushInputBuffer(): Promise<void>;
}
