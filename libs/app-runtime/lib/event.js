module.exports = class {
  publish(event, data) {
    process.send({ type: 'log', body: `@${event} ${data}` })
  }
}
