import * as firebase from "firebase-admin";
import { Platform, DeviceData } from "../../types";

export class FirebasePlatform extends Platform {
    constructor() {
        super();
        firebase.initializeApp();
    }

    async getDeviceData(deviceName: string): Promise<DeviceData> {
        return await firebase
                        .firestore()
                        .collection('devices')
                        .doc(deviceName)
                        .get();
    }

    async setDeviceData(deviceName: string, data: DeviceData) {
        await firebase
                  .firestore()
                  .collection('devices')
                  .doc(deviceName)
                  .set(data);
    }
}
