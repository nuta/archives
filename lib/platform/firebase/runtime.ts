import * as firebase from "firebase-admin";
import { DeviceData } from "../../types";
import { PlatformRuntime } from "../runtime";

export class FirebasePlatformRuntime extends PlatformRuntime {
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
