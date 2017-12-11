import * as fs from "fs";

export type DeviceType = 'sdk' | 'raspberrypi3';

export class DeviceAPI {
    public getDeviceType() {
        if (process.env.MAKESTACK_DEVICE_TYPE) {
            return process.env.MAKESTACK_DEVICE_TYPE as DeviceType;
        }

        if (fs.existsSync("/proc/cpuinfo")) {
            const cpuinfo = fs.readFileSync("/proc/cpuinfo", { encoding: 'utf-8' });
            if (cpuinfo.match(/BCM2837/)) {
                return 'raspberrypi3';
            }
        }

        return 'sdk';
    }
}
