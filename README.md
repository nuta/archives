MakeStack
=========

**[Getting Started](https://github.com/makestack/makestack/blob/master/Documentation/quickstart.md)** | **[API Refernce](https://github.com/makestack/makestack/blob/master/Documentation/api.md)** | **[Change Log](https://github.com/makestack/makestack/blob/master/Documentation/changelog.md)** |**[Tryout Server](https://try-makestack.herokuapp.com/)** | **[CI](https://travis-ci.org/makestack/makestack)**

MakeStack is a software stack for connected devices for super-rapid prototyping that includes:

- **Focus on Developer Experience:** Easy-to-use Node.js API.
- **Batteries-included:** Remote app/os update, remote config management, sending command to device, etc.
- **Integrations:** Incoming/Outgoing Webhook, IFTTT, Slack, and Datadog.
- **A minimalized Linux:** smaller than 32MB including the root file system!
- **Desktop app:** An Electron-based desktop app.
- **Full-featured Backend:** A Rails and Vue.js based server backend.
- **CLI SDK:** A Node.js SDK with CLI frontend to manage apps & devices.
- **Node-RED device side programming:** Enjoy physical computing without coding!
- **Plugins:** Of course we have!
- **Free from vendor lock-in:** Fully open sourced :)

Sample Code
-----------

```javascript
// Load APIs (https://github.com/makestack/makestack/blob/master/Documentation/api.md)
const { Timer, Config, publish } = require('makestack')

// Plugins.
const AQM0802A = require('@makestack/aqm0802a')
const HDC1000 = require('@makestack/hdc1000')

// Initialize device drivers.
const display = new AQM0802A()
const sensor = new HDC1000()

Config.onChange('messages', msg => {
  // print() sends a log message to the server.
  print(`Updating the display message to ${msg}.`)

  // Update the display.
  display.update(msg)
})

// Send temperature and humidity sensed by HDC1000.
Timer.interval(5, () => {
  publish('t', sensor.readTemperature())
  publish('h', sensor.readHumidity())
})
```

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
