const drivelist = require("drivelist");
import { FatalError } from "./types";

export function getDriveSize(devfile: string): Promise<number> {
    return new Promise((resolve, reject) => {
        drivelist.list((error: Error, drives: any[]) => {
            if (error) { reject(error); }

            for (const drive of drives) {
                if (devfile === drive.device) {
                    resolve(drive.size);
                }
            }

            reject(new FatalError(`No such a drive: ${devfile}`));
        });
    });
}

export function getAvailableDrives() {
    return new Promise((resolve, reject) => {
        drivelist.list((error: Error, drives: any[]) => {
            if (error) { reject(error); }

            const availableDrives = drives.filter((drive) => !drive.isSystem);
            resolve(availableDrives);
        });
    });
}
