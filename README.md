makestack
=========
[![Build Status](https://travis-ci.com/seiyanuta/makestack.svg?branch=master)](https://travis-ci.com/seiyanuta/makestack)
[![Test Coverage](https://api.codeclimate.com/v1/badges/d185d1942655beb89ab5/test_coverage)](https://codeclimate.com/github/seiyanuta/makestack/test_coverage)
[![Maintainability](https://api.codeclimate.com/v1/badges/d185d1942655beb89ab5/maintainability)](https://codeclimate.com/github/seiyanuta/makestack/maintainability)

A minimalistic IoT framework for super-rapid prototyping.

- Develop and deploy device-side, server-side, and web app interface all at once.
- Log collection, remote firmware update, remote procedure call, and more.
- Support ESP-32.
- Fully open sourced (CC0 or MIT).

Getting Started
---------------

1. Install MakeStack.
```
$ npm install -g makestack
```

2. Create a new app.
```
$ makestack new helloworld
$ cd helloworld
```

4. Flash a firmware to a device.
```
$ makestack install
```

4. Start a local server for development.
```
$ makestack dev
```

5. Edit source files. MakeStack automatically deploys the app remotely.

Deploying to Firebase
---------------------

1. Update `makestack.production.firebaseProject` in `package.json`.
2. Deploy

```
$ makestack deploy
```

3. Flash a firmware for production.

```
$ makestack install --production
```
