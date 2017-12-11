import * as fs from "fs";
import * as path from "path";
import { AppAPI } from "./api/app";
import { publish } from "./api/event";
import { eprintln, println } from "./api/logging";
import { SerialAPI } from "./api/serial";
import { ConfigAPI } from "./api/config";
import { SubProcessAPI } from "./api/subprocess";
import { TimerAPI } from "./api/timer";
import { dummyGPIO, dummyI2C, dummySPI } from "./dummy";
import { GPIOConstructor, I2CConstructor, SPIConstructor, DeviceType } from "./types";

function detectDeviceType(): DeviceType {
    if (process.env.DEVICE_TYPE) {
        return process.env.DEVICE_TYPE as DeviceType;
    }

    if (fs.existsSync("/proc/cpuinfo")) {
        const cpuinfo = fs.readFileSync("/proc/cpuinfo", { encoding: 'utf-8' });
        if (cpuinfo.match(/BCM2837/)) {
            return 'raspberrypi3';
        }
    }

    return 'sdk';
}

const device = require(`./devices/${detectDeviceType()}`);
export { println, eprintln, publish };
export const GPIO: GPIOConstructor | undefined = device.GPIO || dummyGPIO;
export const I2C: I2CConstructor | undefined = device.I2C || dummyI2C;
export const SPI: SPIConstructor | undefined = device.SPI || dummySPI;
export const Timer = new TimerAPI();
export const Config = new ConfigAPI();
export const App = new AppAPI();
export const SubProcess = new SubProcessAPI();
export const Serial = SerialAPI;
