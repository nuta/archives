import { Platform } from "../types";
import { Platform as FirebasePlatform } from "./firebase";
import { Platform as LocalPlatform } from "./local";

export let platformCtor: any;
if (process.env.MAKESTACK_PLATFORM == "local") {
    platformCtor = LocalPlatform;
} else if (process.env.GCLOUD_PROJECT) {
    platformCtor = FirebasePlatform;
} else {
    throw new Error("cannot detect the platform");
}

export const platform = new platformCtor() as Platform;
