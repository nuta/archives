module.exports = class {
  constructor(loggingApi) {
    this.loggingApi = loggingApi;
  }

  get globals() {
    return {
      publish: this.publish.bind(this)
    }
  }

  reset() {

  }

  publish(event, data) {
    loggingApi.print(`@${event} ${data}`);
  }
}