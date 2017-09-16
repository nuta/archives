const os = require('os');
const fs = require('fs');

const OUTPUT = 'out';
const INPUT  = 'in';

if (os.type() == 'Linux') {
  module.exports = class {
    constructor() {
    }
    
    get globals() {
      return {
        pinMode: this.pinMode.bind(this),
        digitalWrite: this.digitalWrite.bind(this),
        digitalRead: this.digitalRead.bind(this),
        OUTPUT,
        INPUT
      };
    }

    reset() {
      
    }

    pinMode(pin, mode) {
      if (typeof pin !== 'number')
        throw "`pin' must be a number";
        
      this.getSysfsPin(pin);
      fs.writeFileSync(`/sys/class/gpio/export`, `${pin}`);
      fs.writeFileSync(`/sys/class/gpio/gpio${pin}/direction`,
                       (mode == INPUT) ? "in" : "out");
    }
     
    digitalWrite(pin, value) {
      fs.writeFileSync(`/sys/class/gpio/gpio${pin}/value`, value ? "1" : "0");
    }
      
    digitalRead(pin) {
      return fs.readFileSync(`/sys/class/gpio/gpio${pin}/value`, "utf-8") == "1";
    }
  }
} else {
  module.exports = class {
    constructor() { }
    get globals() {
      return {
        pinMode:    this.pinMode.bind(this),
        digitalWrite: this.digitalWrite.bind(this),
        digitalRead:  this.digitalRead.bind(this),
        OUTPUT,
        INPUT
      }
    }

    reset() { }
    pinMode(pin, mode) { }
    digitalWrite(pin, value) { }
    digitalRead(pin) { return 0; }
  }
}
