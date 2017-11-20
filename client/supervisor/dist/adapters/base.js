"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
class AdapterBase {
    constructor() {
        this.onReceiveCallback = () => { };
    }
    onReceive(callback) {
        this.onReceiveCallback = callback;
    }
}
exports.default = AdapterBase;
