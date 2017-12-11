import * as fs from "fs";
import { logger } from './logger';

export function sendToSupervisor(type: string, meta: { [key: string]: string }) {
    if (!process.send) {
        logger.error('process.send it not defined')
        return
    }

    process.send(Object.assign({ type }, meta))
}

export type DeviceType = 'sdk' | 'raspberrypi3';
export function getDeviceType(): DeviceType {
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
