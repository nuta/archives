class AppAPI {
  enableUpdate() {
    process.send({ type: 'setUpdateEnabled', body: true })
  }

  disableUpdate() {
    process.send({ type: 'setUpdateEnabled', body: false })
  }

  onExit(callback) {
    process.on('SIGTERM', () => {
      callback()
    })
  }
}

module.exports = AppAPI
