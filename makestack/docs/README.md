# MakeStack
[![Build Status](https://travis-ci.com/seiyanuta/makestack.svg?branch=master)](https://travis-ci.com/seiyanuta/makestack)

> A minimalistic JavaScript IoT framework for rapid prototyping.

## What is MakeStack?

MakeStack is a server-side/device-side software framework focused on developer experience for rapid connected device protoyping. It offers out-of-the-box features that frees you from troublesome settings and allows you write less code.

## Warning
**MakeStack is in the very early stage. There's a lot of unimplemented language features, APIs, and issues. Help me improve this by filing bugs and submitting Pull requests!**

```js
const app = require("makestack")
const SlackWebClient = require("@slack/web-api").WebClient

app.onEvent("my-sensor-data", async (value) => {
    /* Server-side: post received sensor data on Slack. */
    const slack = new SlackWebClient("XXXXXXXXXXXXXXX")
    cosnt channelId = "C123456"
    await slack.chat.postMessage({
        channel: channelId,
        text: `sensor data: ${value}`,
    })
})

app.onReady((device) => {
    /* Device-side: send sensor data every 15 minutes. */
    while (1) {
        const value = device.analogRead(pin)
        device.publish("my-sensor-data", value)
        device.delayMinutes(15)
    }
})
```

## Features
- **Single-file server-side/device-side programming in modern JavaScript** powered by the [JavaScript to C++ transpiler](device-side-javascript.md).
- **Simplified development workflow:** just remember `flash`, `dev`, and `deploy` command.
- **No user registration required:** deploy everything to your Firebase with just one command.
- **Intuitive JavaScript API.**
- **Remote device firmware update.**
- **Remote device log collection.**

## Documentation
- [Getting Started](getting-started.md)
- [API Reference](api.md)
- [Dos and Don'ts](do-and-donts.md)
- [Device-side JavaScript](device-side-javascript.md)
- [Security](security.md)

## Examples
- [Example apps in the repository](https://github.com/seiyanuta/makestack/tree/master/examples).

## License
[CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/) or [MIT License](https://opensource.org/licenses/MIT). Choose whichever you prefer.
