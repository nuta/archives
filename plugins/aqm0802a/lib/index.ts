export class AQM0802A {
    public bus: I2C;

    constructor(address = 0x3e) {
        this.bus = new I2C({ address });

        for (const cmd of [0x39, 0x14, 0x74, 0x56, 0x6c, 0x0c]) {
            this.bus.write([0x00, cmd]);
        }

        this.clear();
    }

    public async clear() {
        this.bus.write([0x00, 0x01]);
        await Timer.busywait(5 * 1000);
    }

    public async update(text: string) {
        const shortenedText = text.toString().substring(0, 16);

        this.clear();
        for (const [i, ch] of shortenedText.split("").entries()) {
            this.bus.write([0x00, 0x80 + ((i > 7) ? 0x40 + i - 8 : i)]);
            await Timer.busywait(5 * 1000);
            this.bus.write([0x40, ch.charCodeAt(0)]);
            await Timer.busywait(5 * 1000);
        }
    }
}
