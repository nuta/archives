"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const fs = require("fs");
module.exports = class {
    updateOS(imagePath) {
        fs.renameSync(imagePath, '/boot/kernel7.img');
    }
};
