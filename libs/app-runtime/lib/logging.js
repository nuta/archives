module.exports = class {
  print(message) {
    process.send({ type: 'log', body: message + '\n' })
  }

  error(message) {
    this.print(`!${message}`)
  }
}
