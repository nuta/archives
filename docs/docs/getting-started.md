# Getting Started

## Installation
```
$ npm install -g makestack
```

## Prototyping an app
1. Create a app and enter the directory.
```
$ makestack new helloworld
$ cd helloworld
```

2. Connect your device and install the firmware.
```
$ makestack install
```

3. Start the server for development.
```
$ makestack dev
```

4. Edit `device.cc` and `server.js` (refer API documentation for more details). MakeStack
   automatically detects changes to these files, build a new firmware and reload `server.js`,
   and performs remote update to devices.

## Deploying an app
1. Update `makestack.platform` config in `package.json`. Refer firebase documentation for more details.

2. Run `deploy` command.
```
$ makestack deploy
```

3. Install the firmware for production to devices so that they communicate with the deployed app.
```
$ makestack install --production
```

To deploy a new version simply run the same command.

### Caveats
Currently device authentication is not yet implemented and by default MakeStack does **not** provides authentication to API endpoints!
