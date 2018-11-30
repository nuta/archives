const { GPIO } = require('makestack')

export class Led {
    pin: number;
    port: any;
    state: boolean;

    constructor({ pin }: { pin: number }) {
        this.pin = pin
        this.port = new GPIO({ pin, mode: 'out' })

        this.off()
    }

    on() {
        this.port.write(true)
        this.state = true
    }

    off() {
        this.port.write(true)
        this.state = false
    }

    toggle() {
        const newState = !this.state
        this.port.write(newState)
        this.state = newState
    }
}
