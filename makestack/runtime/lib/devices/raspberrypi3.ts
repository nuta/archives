import { LinuxGPIOAPI } from "./linux/gpio";
import { LinuxI2CAPI } from "./linux/i2c";
import { LinuxSPIAPI } from "./linux/spi";
import {
    I2CInterface, SPIInterface, GPIOInterface, SPIMode, SPIOrder
} from "../types";

class GPIO extends LinuxGPIOAPI {
}

const I2C_FILE = "/dev/i2c-1";
class I2C extends LinuxI2CAPI implements I2CInterface {
    constructor(args: { address: number }) {
        super(I2C_FILE, args);
    }
}

const SPI_BUS = "0";
class SPI extends LinuxSPIAPI implements SPIInterface {
    constructor(args: {
        slave?: number, speed?: number, mode: SPIMode, order?: SPIOrder,
        bits?: number, ss?: number, path?: string
    }) {
        super(SPI_BUS, args);
    }
}

export { GPIO, I2C, SPI };
