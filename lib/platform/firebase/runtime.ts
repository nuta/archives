import * as firebase from "firebase-admin";
import { DeviceData, Platform } from "../../types";

export class FirebasePlatform extends Platform {
    constructor() {
        super();
        firebase.initializeApp();
    }

    public async getDeviceData(deviceName: string): Promise<DeviceData> {
        return await firebase
                        .firestore()
                        .collection("devices")
                        .doc(deviceName)
                        .get();
    }

    public async setDeviceData(deviceName: string, data: DeviceData) {
        await firebase
                  .firestore()
                  .collection("devices")
                  .doc(deviceName)
                  .set(data);
    }
}
