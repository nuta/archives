module.exports = class {
  print(message) {
    process.send({ type: 'log', body: message })
  }

  error(message) {
    this.print(`!${message}`)
  }
}
