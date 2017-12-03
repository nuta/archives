type TOnReceiveCallback = (payload: Buffer) => void;

export abstract class AdapterBase {
  public onReceiveCallback: TOnReceiveCallback;
  public abstract async send(payload: Buffer): Promise<void>;
  public abstract getAppImage(version: string): Promise<Buffer>;
  public abstract getOSImage(version: string): Promise<Buffer>;

  constructor() {
    this.onReceiveCallback = () => { };
  }

  public onReceive(callback: TOnReceiveCallback) {
    this.onReceiveCallback = callback;
  }
}
