import { Serial } from "../serial";

/*
*  WebUSB interface is not defined in the current type definition;
*  define it manually for now.
*/
declare global {
    interface Navigator {
        usb: {
            getDevices(): Promise<any[]>;
            requestDevice(args: any): Promise<any>;
        }
    }
}

export class CP2102Driver implements Serial {
    usb: any;

    constructor() {
        this.usb = null
    }

    async open() {
        const filters = [
            { 'vendorId': 0x10c4, 'productId': 0xea60 }
        ]

        const devices = await navigator.usb.getDevices();
        if (devices.length > 0) {
            this.usb = devices[0];
        } else {
            const usb = await navigator.usb.requestDevice({ filters })
            await usb.selectConfiguration(1)
            await usb.claimInterface(0)
            await usb.controlTransferOut({
                requestType: 'vendor',
                recipient: 'device',
                request: 0x00,
                index: 0x00,
                value: 0x01
            })
            await usb.controlTransferOut({
                requestType: 'vendor',
                recipient: 'device',
                request: 0x07,
                index: 0x00,
                value: 0x03 | 0x0100 | 0x0200
            })
            await usb.controlTransferOut({
                requestType: 'vendor',
                recipient: 'device',
                request: 0x01,
                index: 0x00,
                value: 0x384000 / 115200
            })
            this.usb = usb;
        }
    }

    close() {

    }

    async read(length: number) {
        const r = await this.usb.transferIn(1, 64);
        return new Uint8Array(r.data.buffer);
    }

    async flushInputBuffer(): Promise<void> {
    }

    async write(data: Uint8Array) {
        await this.usb.transferIn(1, data);
    }
}
