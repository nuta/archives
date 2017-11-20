"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
const util = require("util");
function log(level, ...messages) {
    const colors = {
        'DEBUG': '35',
        'INFO': '34',
        'ERROR': '31',
        'WARN': '33'
    };
    let body = '';
    for (const message of messages) {
        body += ((typeof message === 'string') ? message : util.inspect(message)) + ' ';
    }
    const time = (new Date()).toTimeString().split(' ')[0];
    const color = colors[level] || '';
    console.log(`[\x1b[1;34msupervisor\x1b[0m ${time} \x1b[${color}m${level}\x1b[0m] ${body}`);
}
function debug(...messages) {
    log('DEBUG', ...messages);
}
exports.debug = debug;
function info(...messages) {
    log('INFO', ...messages);
}
exports.info = info;
function error(...messages) {
    log('ERROR', ...messages);
}
exports.error = error;
function warn(...messages) {
    log('WARN', ...messages);
}
exports.warn = warn;
