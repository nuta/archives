import { LinuxGPIOAPI } from "./linux/gpio";
import { LinuxI2CAPI } from "./linux/i2c";
import { LinuxSPIAPI } from "./linux/spi";
import { I2CInterface, SPIInterface } from "../types";

class GPIO extends LinuxGPIOAPI {
}

const I2C_FILE = "/dev/i2c-1";
class I2C extends LinuxI2CAPI implements I2CInterface {
    get path() {
        return I2C_FILE;
    }
}

const SPI_BUS = "0";
class SPI extends LinuxSPIAPI implements SPIInterface {
    get bus() {
        return SPI_BUS;
    }
}

export { GPIO, I2C, SPI };
