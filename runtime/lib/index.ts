import * as path from "path";
import { AppAPI } from "./api/app";
import { publish } from "./api/event";
import { eprintln, println } from "./api/logging";
import { SerialAPI } from "./api/serial";
import { ConfigAPI } from "./api/config";
import { SubProcessAPI } from "./api/subprocess";
import { TimerAPI } from "./api/timer";


const device = require(`./devices/${ process.env.MAKESTACK_DEVICE_TYPE || 'sdk'}`);
export const GPIO = device.GPIO;
export const I2C = device.I2C;
export const SPI = device.SPI;

export { println, eprintln, publish };
export const Timer = new TimerAPI();
export const Config = new ConfigAPI();
export const App = new AppAPI();
export const SubProcess = new SubProcessAPI();
export const Serial = SerialAPI;
