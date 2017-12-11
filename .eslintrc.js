module.exports = {
  "extends": "standard",
  "globals": {
    I2C: false,
    SPI: false,
    GPIO: false,
    Serial: false,
    Timer: false,
    Logging: false,
    Event: false,
    Config: false,
    plugin: false
  },
  "env": {
    "es6": true,
    "browser": true,
    "mocha": true
  },
  "rules": {
    "indent": ["error", 2, { "SwitchCase": 1 }],
    "space-before-function-paren": ["error", "never"]
  }
}
