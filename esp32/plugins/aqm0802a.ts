modules['@makestack/aqm0802a'] = class AQM0802A {
    public bus: any;

    constructor(address = 0x3e) {
        this.bus = new I2C({ address });

        for (const cmd of [0x39, 0x14, 0x74, 0x56, 0x6c, 0x0c]) {
            this.bus.writeSync([0x00, cmd]);
        }

        this.clear();
    }

    public clear() {
        this.bus.writeSync([0x00, 0x01]);
        Timer.busywait(50);
    }

    public update(text: string) {
        const shortenedText = text.toString().substring(0, 16);

        this.clear();
        for (let i = 0; i < shortenedText.length; i++) {
            this.bus.writeSync([0x00, 0x80 + ((i > 7) ? 0x40 + i - 8 : i)]);
            Timer.busywait(50);
            this.bus.writeSync([0x40, shortenedText[i].charCodeAt(0)]);
            Timer.busywait(50);
            i++;
        }
    }
}
