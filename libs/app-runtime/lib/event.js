module.exports = class {
  constructor(logging) {
    this.logging = logging
  }

  get globals() {
    return {
      Event: {
        publish: this.publish.bind(this)
      }
    }
  }

  reset() {

  }

  publish(event, data) {
    this.logging.print(`@${event} ${data}`)
  }
}
