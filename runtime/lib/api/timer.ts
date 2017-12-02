export class TimerAPI {
  public interval(interval, callback) {
    setInterval(callback, interval * 1000);
  }

  /* Don't awit this function! It won't be resolved nor rejected forever. */
  public async loop(callback) {
    while (true) {
      await callback();
    }
  }

  public delay(duration, callback) {
    setTimeout(callback, duration * 1000);
  }

  public sleep(duration) {
    return new Promise((resolve, reject) => {
      setTimeout(resolve, duration * 1000);
    });
  }

  public busywait(usec) {
    if (Math.pow(10, 9) /* 1 sec */ <= usec) {
      throw new Error("Too long busy wait duration.");
    }

    const start = process.hrtime();

    while (true) {
      const [secDiff, nanosecDiff] = process.hrtime(start);
      if (secDiff > 0 || nanosecDiff > usec * 1000) {
        return;
      }
    }
  }
}
