# Writing a Plugin

*Plugin* is a pure Node.js package such as device driver for I2c peripherals. Let's
create a plugin named *led-blinker* and learn how to write a new plugin.

Requirements
-------------

- A macOS or Linux machine with Node.js version 8.x or higher
- [MakeStack CLI developer tools](https://www.npmjs.com/package/makestack-sdk)

Setup
------

1. Create plugin template files.

```bash
makestack new-plugin led-blinker
cd led-blinker
```

2. Create an app for debugging the plugin.

```bash
yarn create-app
```

Writing code
------------

You'll see there files in the directory:

- **lib/index.ts:** A plugin source code.
- **app.js:** An app code for debugging and demonstrating your plugin.

2. Edit `lib/index.ts`.

```typescript
import { GPIO } from 'makestack';

export class LEDBlinker {
    private instance: GPIO;
    private interval: number;

    constructor(pin: number, interval: number = 0.5) {
        this.instance = new GPIO({ pin, mode: 'out' });
        this.interval = interval;
    }

    public start() {
        let state = true;

        this.timer = setInterval(() => {
            this.instance.write(state);
            state = !state; // invert the state
        }, this.interval)
    }

    public stop() {
        clearInterval(this.timer)
    }
}
```

3. Edit `app.js`.

```js
// JavaScript
const { LEDBlinker } = require('.')

const blinker = LEDBlinker({ pin: 26 })
blinker.start()
```

4. Transpile plugin source code.

```bash
yarn build
```

5. Deloy `app.js` including compiled plugin files. **Don't forget to transpile source code before deploying!**

```bash
makestack deploy
```

That's it!

Publishing a plugin
-------------------

Send us a pull request to add your cool plugin into [official plugins](https://github.com/makestack/makestack/tree/master/plugins)!
