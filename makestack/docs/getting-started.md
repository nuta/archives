# Getting Started

## Installing MakeStack
```
$ npm install -g makestack
```

## Create a new app
The `new` command generates a app directory. The `--firebase` option generates Firebase configuration
files as well:

```
$ makestack new --firebase hello
$ cd hello
```

If you prefer TypeScript (**highly reccomended!**), specify `--typescript` option:

```
$ makestack new --typescript --firebase hello
$ cd hello
```

## Install the firmware
Connect your device using a serial port and run the `flash` command. It automatically detects the serial port:
```
$ makestack flash
```

In case there're multiple serial port device files, specify which one to use by `--device` option:
```
$ makestack flash --device /dev/ttyUSB1
```

## Start development
The `dev` command communicates with the device over a serial port, automatically rebuilds the firmware
restarts the server when you updated the code, and automatically flashes a new firmware:

```
$ makestack dev
```

## Write code
See [API reference](api) and [Examples](https://github.com/seiyanuta/makestack/tree/master/examples).

```
$ vim app.js
```

## Deploy to Firebase
```
$ WIFI_PASSWORD=YOUR-WIFI-PASSWORD makestack flash \
    --adapter wifi \
    --wifi-ssid YOUR-WIFI-SSID \
    --server-url http://us-central1-YOUR-FIREBASE-PROJECT.cloudfunctions.net/api \
    --heartbeat-interval 900
```

```
$ makestack deploy
```
