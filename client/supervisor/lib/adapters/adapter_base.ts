export abstract class AdapterBase {
  onReceiveCallback: (Buffer) => void;
  abstract async send(payload: Buffer);
  abstract getAppImage(version: string): Promise<Buffer>;
  abstract getOSImage(version: string): Promise<Buffer>;

  constructor() {
    this.onReceiveCallback = () => { }
  }

  onReceive(callback) {
    this.onReceiveCallback = callback
  }
}
