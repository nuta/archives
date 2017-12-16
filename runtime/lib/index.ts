import * as fs from "fs";
import { AppAPI } from "./api/app";
import { DeviceAPI } from "./api/device";
import { publish } from "./api/event";
import { eprintln, println } from "./api/logging";
import { SerialAPI } from "./api/serial";
import { ConfigAPI } from "./api/config";
import { SubProcessAPI } from "./api/subprocess";
import { TimerAPI } from "./api/timer";
import { dummyGPIO, dummyI2C, dummySPI } from "./dummy";
import { getDeviceType } from "./helpers";
import { GPIOConstructor, I2CConstructor, SPIConstructor } from "./types";
import { logger } from "./logger";

const device = require(`./devices/${getDeviceType()}`);
export { println, eprintln, publish, logger };
export const GPIO: GPIOConstructor = device.GPIO || dummyGPIO;
export const I2C: I2CConstructor = device.I2C || dummyI2C;
export const SPI: SPIConstructor = device.SPI || dummySPI;
export const Timer = new TimerAPI();
export const Config = new ConfigAPI();
export const App = new AppAPI();
export const Device = new DeviceAPI();
export const SubProcess = new SubProcessAPI();
export const Serial = SerialAPI;

if (process.env.MAKESTACK_APP) {
    process.on('unhandledRejection', (reason, p) => {
        console.log('runtime: unhandled rejection:\n', reason, '\n\n', p)
        console.log('runtime: exiting...')
        process.exit(1)
    })

    process.on('message', (data) => {
        switch (data.type) {
            case 'initialize':
                logger.info(`initialize message: configs=${JSON.stringify(data.configs)}`)
                Config.update(data.configs)
                break

            case 'configs':
                logger.info(`configs message: configs=${JSON.stringify(data.configs)}`)
                Config.update(data.configs)
                break

            default:
                logger.info('unknown ipc message: ', data)
        }
    })

    logger.info("waiting for `initialize' message from Supervisor...");
}
