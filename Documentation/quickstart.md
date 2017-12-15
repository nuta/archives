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

You need an user account on MakeStack Server. You can use [a demo server](https://try-makestack.herokuapp.com/) for free or [deploy your own MakeStack Server](https://github.com/seiyanuta/makestack/blob/master/Documentation/guides/heroku.md).

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

1. Create an app and `cd(1)` into the generated directory.

```bash
makestack new --register helloworld
cd helloworld
```

You'll see there files in the directory:

- **app.yaml:** A MakeStack app config file. Leave it as it is for now.
- **app.js:** A JavaScript (Node.js) script to be run on the device.

2. Edit `app.js`.

```js
/*
   Load MakeStack APIs.
   Refer: https://github.com/seiyanuta/makestack/blob/master/Documentation/api.md
*/
const { Timer, println } = require('makestack/@runtime')

/* Timer.interval(n, callback) calls `callback` every `n` seconds. */
Timer.interval(3, () => {
    /* println(str) adds `str` to the log buffer. The log buffer will be sent to
       MakeStack Server in a heartbeat. */
    println('Hello World!')
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

That's it!
