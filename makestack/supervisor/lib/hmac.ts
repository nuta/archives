import * as crypto from "crypto";

export function computeHMAC(deviceSecret: string, data: Buffer | string) {
    const hmac = crypto.createHmac("sha256", deviceSecret);
    hmac.update(data);
    return hmac.digest("hex");
}

export function verifyMessageHMAC(deviceSecret: string, payload: Buffer, hmac: string, timestamp: string) {
    if (typeof timestamp !== "string") {
        console.error("timestamp is not set");
        return false;
    }

    const diffFromTimestamp = Math.abs(((new Date()).getTime() - (new Date(timestamp)).getTime()));
    if (diffFromTimestamp > 5 * 60 * 1000 /* msec */) {
        console.error("invalid timestamp");
        return false;
    }

    if (typeof hmac !== "string") {
        console.error("hmac is not set");
        return false;
    }

    if (hmac !== computeHMAC(deviceSecret, payload)) {
        console.error("invalid hmac");
        return false;
    }

    return true;
}

export function verifyImageHMAC(deviceSecret: string, hmac: string, image: Buffer) {
    if (!hmac) {
        return false;
    }

    const hash = crypto.createHash("sha256");
    hash.update(image);
    const shasum = hash.digest("hex");

    return (hmac === computeHMAC(deviceSecret, shasum));
}
