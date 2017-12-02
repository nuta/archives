export abstract class AdapterBase {
  public onReceiveCallback: (Buffer) => void;
  public abstract async send(payload: Buffer);
  public abstract getAppImage(version: string): Promise<Buffer>;
  public abstract getOSImage(version: string): Promise<Buffer>;

  constructor() {
    this.onReceiveCallback = () => { };
  }

  public onReceive(callback) {
    this.onReceiveCallback = callback;
  }
}
