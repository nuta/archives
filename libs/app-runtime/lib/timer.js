module.exports = class {
  constructor() {
    this.timers = [];
  }

  get globals() {
    return {
      loop: this.loop.bind(this)
    };
  }

  reset() {
    for (let i = 0; i < this.timers.length; i++) {
      clearInterval(this.timers[i]);
    }

    this.timers = [];
  }

  loop(interval, callback) {
    if (interval == 0)
      interval = 1;
  
    let timer = setInterval(callback, interval * 1000);
    this.timers.push(timer);
  }
}
