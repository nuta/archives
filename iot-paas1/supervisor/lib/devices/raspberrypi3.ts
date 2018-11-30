import * as fs from "fs";
import { DeviceBase } from "./device_base";

export class Device extends DeviceBase {
    public updateOS(image: Buffer) {
        fs.writeFileSync("/boot/kernel7.img", image);
    }
}
