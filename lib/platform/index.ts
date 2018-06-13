import * as path from "path";
import { Platform } from "../types";
import * as firebase from "./firebase";
import * as local from "./local";

export function instantiatePlatform(): Platform {
    let platformCtor: any;
    if (process.env.MAKESTACK_PLATFORM == "local") {
        platformCtor = local.Platform;
    } else if (process.env.GCLOUD_PROJECT) {
        platformCtor = firebase.Platform;
    } else {
        throw new Error("cannot detect the platform");
    }

    return new platformCtor() as Platform;
}

export function loadPlatform(platform: string): any {
    return require(path.resolve(__dirname, `./${platform}`));
}
