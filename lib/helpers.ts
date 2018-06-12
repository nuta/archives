import * as fs from "fs-extra";

export function guessSerialFilePath(): string | null {
    const patterns = [
        /\Acu\.usbserial-[0-9]+\z/,
        /\AttyUSB[0-9]\z/
    ];

    const candidates = fs.readdirSync("/dev")
        .filter(file =>
            patterns.some(pat => pat.exec(file) !== null)
        );

    if (candidates.length === 1) {
        return candidates[0];
    } else {
        return null;
    }
}
