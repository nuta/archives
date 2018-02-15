---
title: Quickstart
---

**Currently MakeStack is severly unstable. You will have to debug MakeStack to try. Good luck!**

Requirements
-------------

- A macOS or Linux machine with Node.js version 8.x or higher
- Raspberry Pi 3
- Internet connection: Ethernet or Wi-Fi (WPA2-PSK)

Creating an account
-------------------

You need an user account on MakeStack Server. You can use [a demo server](https://try-makestack.herokuapp.com/)
for free or [deploy your own MakeStack Server](https://github.com/makestack/makestack/blob/master/Documentation/guides/heroku.md).

Installing SDK
--------------

1. Install MakeStack SDK by npm.
```bash
npm install -g makestack
```

2. Login and save credentials. Credentials are stored in `~/.makestack`.
```bash
makestack login
```

Installing OS
-------------

1. Insert a SD card to flash the os image. Please note that contents in the SD card
   will be erased.

2. Get the drive name of the SD card.
```
makestack list-drives
```

3. Install the OS to the drive.
```bash
DEVICE_NAME="my-raspi3"
makestack install --name $DEVICE_NAME --type raspberrypi3 --drive <DRIVE_NAME> \
  --wifi-ssid <WIFI_SSID> --wifi-password <WIFI_PASSWORD> --wifi-country <WIFI_COUNTRY>
```

`<WIFI_COUNTRY>` is a [ISO 3166-1 alpha-2](https://en.wikipedia.org/wiki/ISO_3166-1_alpha-2) contry
code. `US` for the United States, for example.


You don't have to specify the Wi-Fi credential if you want to use Ethernet instead:
```bash
DEVICE_NAME="my-raspi3"
makestack install --name $DEVICE_NAME --type raspberrypi3 --drive <drive-name>
```

4. Power on the Raspberry Pi with the SD card inserted.

5. Make sure that the device is online.

```bash
makestack device list
```

From now on the device is managed completely remotely by MakeStack Server!


Creating the your first app
---------------------------
**In this document we use CLI tools. You can do same things by Web UI If you prefer.**

1. Create an app and `cd(1)` into the generated directory.

```bash
makestack new --register helloworld
cd helloworld
```

You'll see there files in the directory:

- **package.json:** A MakeStack app config file. Leave it as it is for now.
- **app.js:** A JavaScript (Node.js) script to be run on the device.

2. Edit `app.js`.

```js
/*
   Load MakeStack APIs.
   Refer: https://github.com/makestack/makestack/blob/master/Documentation/api.md
*/
const { Timer, print } = require('makestack')

/* Timer.interval(n, callback) calls `callback` every `n` seconds. */
Timer.interval(3, () => {
    /* print(str) adds `str` to the log buffer. The log buffer will be sent to
       MakeStack Server in a heartbeat. */
    print('Hello World!')
})
```

3. Add the device to the app.
```bash
makestack add-device $DEVICE_NAME
```

4. You're now ready for deploy! Let's deloy the Hello World app.
```bash
makestack deploy
```

5. Take a look at log messages sent from the device to the MakeStack Server.
```
makestack log
```

Integrating with Slack
----------------------
To send a data to server, simply use `publish()` API:

```js
// app.js
const { publish } = require('makestack')
Timer.interval(3, () => {
  /* publish(event_name, value) */
  publish('random number', Math.random())
})
```

Sent *events* by `publish()` are forwarded to integrated services such as IFTTT,
Slack, and webhooks. Let's add a new integration with Slack!

```bash
makestack integration add --service slack --webhook-url <SLACK_INCOMING_WEBHOOK_URL>
```

Control a light remotely
-------------------------
Config API provides remote configuration system. *Config* is a readonly value stored
in MakeStack Server and automatically sent to devices. There are two scopes of config: device
config and app config. If same config name exists in either device config and app config, device
config is sent to the device.


To watch changes to a config, use `Config.onChange(config_name, callback)`:

```js
// app.js

/* The pin number depends on the device. If you are using Raspberry Pi3, refer:
   https://github.com/makestack/makestack/blob/master/Documentation/guides/raspberrypi3.md
*/
const light = new GPIO({ pin: 23, mode: 'out' })

Config.onChange('state', state => {
  light.write(state === 'on')
})
```

To update config, use `maketack config` command to change app config or `makestack device-config` to
change device config:

```bash
makestack device-config set state on
```

Using a plugin
--------------

Plugins make it easier to create an app. In this section, we learn how to use plugin and
create a temperature sensor using [HDC1008](https://www.adafruit.com/product/2635) and its
device driver plugin.

To use plugin simply run `add-plugin` and `require()` it in `app.js`:

```
makestack add-plugin @makestack/hdc1000
```

In Web Editor third-party plugins (ones without `@makestack/` prefix) are not available.

```js
const { HDC1000 } = require('@makestack/hdc1000')

/* Send temperature and humidity every 5 seconds */
const sensor = new HDC1000()
Timer.interval(5, () => {
  publish('t', sensor.readTemperature())
  publish('h', sensor.readHumidity())
})
```

References
-----------
- [API Reference](https://github.com/makestack/makestack/blob/master/Documentation/api.md)
- [Writing a Plugin](https://github.com/makestack/makestack/blob/master/Documentation/guides/writing-plugin.md)
