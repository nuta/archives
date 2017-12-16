---
title: Writing a Plugin
---

**TODO: This documention is out-of-date.**

Requirements
-------------

- A macOS or Linux machine with Node.js version 8.x or higher

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

Creating the your first plugin
------------------------------

1. Create an plugin and `cd(1)` into the generated directory.

```bash
makestack new-plugin led-blinker
cd led-blinker
```

You'll see there files in the directory:

- **plugin.yaml:** A MakeStack plugin config file. Leave it as it is for now.
- **lib/index.ts:** A TypeScript (Node.js) script.
- **example:** A sample MakeStack app to test the plugin.

2. Edit `lib/index.ts`.

```typescript
// TypeScript
import { GPIO } from '@makestack/runtime';

export class LEDBlinker {
    private instance: GPIO;
    private interval: number;

    constructor(pin: number, interval: number = 0.5) {
        this.instance = new GPIO({ pin, mode: 'out' });
        this.interval = interval;
    }

    public start() {
        let state = false;

        this.timer = setInterval(() => {
            this.instance.write(state);
            state = !state; // invert the state
        })
    }

    public stop() {
        clearInterval(this.timer)
    }
}
```

3. Edit `example/app.js`.

```js
// JavaScript
const { LEDBlinker } = require('@makestack/led-blinker')

const blinker = LEDBlinker({ pin: 22, interval: 2 })
blinker.start()
```

4. Deloy the example app.
```bash
cd example && makestack deploy
```

Publishing a plugin
-------------------

Add it
