Homekit
========
A HomeKit plugin for MakeStack apps.

**Currently this plugin is not available since its dependency,
`mdns`, depends on external libraries so we cannot cross-compile. This issue will be resolved by this PR: https://github.com/KhaosT/HAP-NodeJS/pull/495**

```javascript
const { Light } = require('@makestack/homekit')
const lightState = false;

const light = new Light({
    name: 'my-light', // The device name to be displayed on iOS.
    id: '12:12:12:12:12:12', // A MAC-address-like ID. This must be unique.
    pin: '111-11-111' // A secret pin code used for authentication.
})

light.onOnGet(() => {
    return lightState
})

light.onOnSet(newState => {
    console.log(`turning the light ${newState ? 'on' : 'off'}`)
    lightState = newState
})

light.start()
```

License
-------
See `LICENSE`.
