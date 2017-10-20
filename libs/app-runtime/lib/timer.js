module.exports = class {
  loop(interval, callback) {
    if (interval < 0.1) {
      interval = 0.1
    }

    setInterval(callback, interval * 1000)
  }
}
