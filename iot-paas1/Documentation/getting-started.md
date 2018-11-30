Requirements
-------------

- A macOS or Linux machine with Node.js version 8.x or higher
- Raspberry Pi 3

Creating an account
-------------------

1. Create an user account. You can use [MakeStack Cloud](https://makestack.cloud)
for free or [deploy your own MakeStack Server](https://github.com/makestack/makestack/blob/master/Documentation/guides/heroku.md).
2. Confirm your email address and create a new app named **helloworld** (or whatever you prefer).
3. You will be redirected to the editor page.

Installing Agent
----------------
In your Raspberry Pi, install and set up a CLI device agent:

```bash
npm install -g makestack-sdk
makestack register --app helloworld my-raspi
```

And start agent:
```bash
makestack run
```

From now on the device is managed completely remotely by MakeStack!

You also can use MakeStack Linux, an experimental small linux distribution made
for MakeStack if you don't want to set up a Linux environment like Raspbian. To
install MakeStack Linux use
MakeStack Desktop.

Deploying a first app
---------------------

Let's say hello from your Raspberry Pi! Copy and paste the following code to the editor and click **Deploy** button.

```js
const { Timer, print } = require('makestack')

/*
 * Timer.interval(n, callback) calls `callback` every `n` seconds. Of course you can use
 * setInterval instead if you prefer.
 */
Timer.interval(3, () => {
    /* print(str) adds `str` to the log buffer. The log buffer will be sent to
       MakeStack Server in a heartbeat. */
    print('Hello World!')
})
```

Click **Open Log** bar in the bottom of editor page and confirm that `Hello World!` message is being received!

Integrating with Slack
----------------------
To send a data to server, simply use `publish()` API:

```js
// app.js
const { publish } = require('makestack')
Timer.interval(3, () => {
  /* publish(eventName: string, value: string | number) */
  publish('random number', Math.random())
})
```

`publish()` emits a *event*, a small data to be forwarded to integrated services such as IFTTT,
Slack, and your own webhooks. Let's add a new integration with Slack!

1. Open **Settings > Integrations** and copy and paste your Slack incoming webhook URL to the form.
2. Deploy the above code.
3. Open Slack and enjoy random numbers sent from the device.

Control a light remotely
-------------------------
Config API provides remote configuration system. *Config* is a readonly value stored
in MakeStack Server and automatically synchronized with devices. There are two scopes
of config: device config and app config. If same config name exists in either device
config and app config, device config is sent to the device.

To watch changes to a config, use `Config.onChange(configName, callback)`:

```js
//
// `pin' number in BCM fashion. Connect a LED to BCM26 of your Raspberry Pi 3:
// https://pinout.xyz/pinout/pin37_gpio26
//
const light = new Led({ pin: 26 })

Config.onChange('led-state', state => {
  if (state === 'on')
   light.on()
  else
    light.off()
})
```

1. Connect a LED to [BCM26 GPIO port](https://pinout.xyz/pinout/pin37_gpio26) on your Raspberry Pi3.
2. Deploy the above code.
3. Open **Settings > Config** and add a config named **led-state** with value `on`.
4. Wait seconds until the config is synchronized.
5. Try changing config to `off` and `on` and enjoy controlling the LED remotely.

Using a plugin
--------------

Plugins make it easier to create an app. In this section, we learn how to use plugin and
create a temperature sensor using [HDC1008](https://www.adafruit.com/product/2635) and its
device driver plugin.

```js
const { publish } = require('makestack')
const { HDC1000 } = require('@makestack/hdc1000')

/* Instantinate a HDC1000 device driver. */
const sensor = new HDC1000()

Timer.interval(5, () => {
  publish('temperature', sensor.readTemperature())
  publish('humidity', sensor.readHumidity())
})
```

1. Connect a HDC1000 to the I2C port ([BCM2 and BCM3](https://pinout.xyz/pinout/i2c)) your Raspberrry Pi3.
2. Deploy the above code.
3. Click **Show Log** bar in the bottom of editor page and confirm that the device sends temperature and humidity events.

What's next?
------------
- [API Reference](api)
- [Writing a Plugin](writing-plugin)
