export type OnReceiveCallback = (payload: Buffer) => Promise<void>;

export abstract class AdapterBase {
    public onReceiveCallback: OnReceiveCallback;
    public abstract async connect(): Promise<void>;
    public abstract async send(payload: Buffer): Promise<void>;
    public abstract getAppImage(version: string): Promise<Buffer>;
    public abstract getOSImage(version: string): Promise<Buffer>;

    constructor() {
        this.onReceiveCallback = () => Promise.resolve();
    }

    public onReceive(callback: OnReceiveCallback) {
        this.onReceiveCallback = callback;
    }
}
