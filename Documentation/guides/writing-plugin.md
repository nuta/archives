# Writing a Plugin

Requirements
-------------

- A macOS or Linux machine with Node.js version 8.x or higher
- [MakeStack CLI developer tools](https://www.npmjs.com/package/makestack-sdk)

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

3. Edit `example/app.js`.

```js
// JavaScript
const { LEDBlinker } = require('@makestack/led-blinker')

const blinker = LEDBlinker({ pin: 22 })
blinker.start()
```

4. Deloy the example app.

```bash
cd example && makestack deploy
```

Publishing a plugin
-------------------

[Publish the plugin as a npm package](https://docs.npmjs.com/getting-started/publishing-npm-packages) or
send us a pull request that add your cool plugin into [official plugins](https://github.com/makestack/makestack/tree/master/plugins).
