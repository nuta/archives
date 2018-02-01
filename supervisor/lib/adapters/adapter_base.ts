export type OnReceiveCallback = (payload: Buffer) => Promise<void>;

export abstract class AdapterBase {
    public onReceiveCallback: OnReceiveCallback;
    public abstract async connect(): Promise<void>;
    public abstract async send(payload: Buffer): Promise<void>;
    public abstract getAppImage(version: number): Promise<Buffer>;
    public abstract getOSImage(version: number): Promise<Buffer>;

    constructor() {
        this.onReceiveCallback = () => Promise.resolve();
    }

    public onReceive(callback: OnReceiveCallback) {
        this.onReceiveCallback = callback;
    }
}
