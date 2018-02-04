// Webpack loads this file instead of `node.ts`.
import { Serial } from "./serial";

export class NodeSerialDriver implements Serial {
    constructor() {}
    async open(args: any) {}
    close() {}

    async read(length: number) {
        return Promise.resolve(new Uint8Array([]));
    }

    async write(data: any) {}
}
