import { AppAPI } from "./api/app";
import { DeviceAPI } from "./api/device";
import { publish } from "./api/event";
import { eprintln, println } from "./api/logging";
import { SerialAPI } from "./api/serial";
import { ConfigAPI } from "./api/config";
import { SubProcessAPI } from "./api/subprocess";
import { TimerAPI } from "./api/timer";
import { dummyGPIO, dummyI2C, dummySPI } from "./dummy";
import { GPIOConstructor, I2CConstructor, SPIConstructor } from "./types";

export const Device = new DeviceAPI();

const device = require(`./devices/${Device.getDeviceType()}`);
export { println, eprintln, publish };
export const GPIO: GPIOConstructor = device.GPIO || dummyGPIO;
export const I2C: I2CConstructor = device.I2C || dummyI2C;
export const SPI: SPIConstructor = device.SPI || dummySPI;
export const Timer = new TimerAPI();
export const Config = new ConfigAPI();
export const App = new AppAPI();
export const SubProcess = new SubProcessAPI();
export const Serial = SerialAPI;
