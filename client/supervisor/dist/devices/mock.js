"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const fs = require("fs");
const os = require("os");
const path = require("path");
const device_1 = require("./device");
class MockDevice extends device_1.default {
    updateOS(imagePath) {
        console.log('os image:', fs.readFileSync(imagePath, { encoding: 'utf-8' }));
        fs.renameSync(imagePath, path.join(os.homedir(), '.makestack/mock-os-update-test.img'));
    }
}
exports.default = MockDevice;
