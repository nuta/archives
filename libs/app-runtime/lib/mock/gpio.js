const os = require('os');
const fs = require('fs');

const OUTPUT = 'out';
const INPUT  = 'in';

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