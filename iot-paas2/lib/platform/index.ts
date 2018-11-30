import { FirebasePlatformRuntime } from "./firebase/runtime";
import { FirebasePlatformSdk } from "./firebase/sdk";
import { LocalPlatformRuntime } from "./local/runtime";
import { LocalPlatformSdk } from "./local/sdk";
import { PlatformRuntime } from "./runtime";
import { PlatformSdk } from "./sdk";

let runtimeInstance: PlatformRuntime;

export function getRuntimeInstance(): PlatformRuntime {
    if (!runtimeInstance) {
        switch (process.env.MAKESTACK_PLATFORM) {
            case "local":
                runtimeInstance = new LocalPlatformRuntime();
                break;
            case "firebase":
                runtimeInstance = new FirebasePlatformRuntime();
                break;
            default:
                throw new Error(`MAKESTACK_PLATFORM is not set.`);
        }
    }

    return runtimeInstance;
}

let sdkInstance: PlatformSdk;

export function getSdkInstance(platform: string): PlatformSdk {
    if (!sdkInstance) {
        switch (platform) {
            case "local":
                sdkInstance = new LocalPlatformSdk();
                break;
            case "firebase":
                sdkInstance = new FirebasePlatformSdk();
                break;
            default:
                throw new Error(`Unknown platform \`${platform}'.`);
        }
    }

    return sdkInstance;
}
