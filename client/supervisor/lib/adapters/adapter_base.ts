export default class AdapterBase {
  onReceiveCallback: (Buffer) => void;

  constructor() {
    this.onReceiveCallback = () => { }
  }

  onReceive(callback) {
    this.onReceiveCallback = callback
  }
}
