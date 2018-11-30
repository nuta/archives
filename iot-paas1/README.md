[![MakeStack](https://cdn.rawgit.com/makestack/media/master/banner.png)](https://makestack.org)

---

MakeStack is a complete software stack for connected devices featuring:

- JavaScript with intuitive APIs.
- Supports Raspberry Pi 3 and ESP-32.
- Remote app/os update, remote config management, remote device log collection.
- Electron-based esktop app.
- Rails-based backend server that [can be easiliy deployed to Heroku](https://makestack.org/documentation/#/guides/heroku).
- Easy integrations with IFTTT, ThingSpeak, and Webhooks.
- Fully open sourced (CC0/MIT).

```javascript
//
//  A simple weather station app.
//
const { Timer, Config, publish, print } = require('makestack')
const HDC1000 = require('@makestack/hdc1000')

const led = new GPIO({ pin: 23, mode: 'out' })
const sensor = new HDC1000()

Config.onChange('weather', value => {
  print('Received a change to weather config; manipulating the LED!')
  led.write(value === 'rainy')
})

// Send temperature and humidity sensed by HDC1000.
Timer.interval(5, () => {
  publish('t', sensor.readTemperature())
  publish('h', sensor.readHumidity())
})
```

Documentation
-------------

- **[Getting Started](https:///makestack.org/documentation/#/getting-started)**
- **[API Reference](https://makestack.org/documentation/#/api)**
- **[Guides](https://makestack.org/documentation/#/guides)**

Code Status
-----------

### CI
[![Build Status](https://travis-ci.org/makestack/makestack.svg?branch=master)](https://travis-ci.org/makestack/makestack)

### Npm Dependencies

| Component | Dependencies | devDependencies |
| ---- | ---- | ---- |
| Runtime | [![dependencies Status](https://david-dm.org/makestack/makestack/status.svg?path=runtime)](https://david-dm.org/makestack/makestack?path=runtime) |[![dependencies Status](https://david-dm.org/makestack/makestack/status.svg?path=runtime&type=dev)](https://david-dm.org/makestack/makestack?path=runtime&type=dev) |
| Supervisor | [![dependencies Status](https://david-dm.org/makestack/makestack/status.svg?path=supervisor)](https://david-dm.org/makestack/makestack?path=supervisor) |[![dependencies Status](https://david-dm.org/makestack/makestack/status.svg?path=supervisor&type=dev)](https://david-dm.org/makestack/makestack?path=supervisor&type=dev) |
| SDK | [![dependencies Status](https://david-dm.org/makestack/makestack/status.svg?path=sdk)](https://david-dm.org/makestack/makestack?path=sdk) |[![dependencies Status](https://david-dm.org/makestack/makestack/status.svg?path=sdk&type=dev)](https://david-dm.org/makestack/makestack?path=sdk&type=dev) |
| Desktop | [![dependencies Status](https://david-dm.org/makestack/makestack/status.svg?path=desktop)](https://david-dm.org/makestack/makestack?path=desktop) |[![dependencies Status](https://david-dm.org/makestack/makestack/status.svg?path=desktop&type=dev)](https://david-dm.org/makestack/makestack?path=desktop&type=dev) |
| UI | [![dependencies Status](https://david-dm.org/makestack/makestack/status.svg?path=ui)](https://david-dm.org/makestack/makestack?path=ui) |[![dependencies Status](https://david-dm.org/makestack/makestack/status.svg?path=ui&type=dev)](https://david-dm.org/makestack/makestack?path=ui&type=dev) |


License
-------
CC0 or MIT. See `LICENSE.md`.
