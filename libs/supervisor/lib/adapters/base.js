class AdapterBase {
  constructor() {
    this.onReceiveCallback = () => { }
  }

  onReceive(callback) {
    this.onReceiveCallback = callback
  }
}

module.exports = AdapterBase
