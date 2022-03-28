import * as fs from "fs";
import * as path from "path";
import { UserError } from "../helpers";

export type Validator =
    | RegExp
    | number // Caporal flags
    | ((arg: string) => any);

export interface ArgDefinition {
    name: string;
    desc: string;
    validator?: Validator;
    default?: any;
}

export interface OptDefinition {
    name: string;
    desc: string;
    validator?: Validator;
    default?: any;
    required?: boolean;
}

export interface Args {
    [name: string]: any;
}
export interface Opts {
    [name: string]: any;
}

export abstract class Command {
    public static command: string;
    public static desc: string;
    public static args: ArgDefinition[];
    public static opts: OptDefinition[];
    public static watchMode: boolean;
    public abstract async run(args: Args, opts: Opts): Promise<void>;
}

export const validateAppDir: Validator = (appDir: string): any => {
    if (!fs.existsSync(appDir)) {
        throw new UserError("The app directory does not exist");
    }

    appDir = path.resolve(appDir);
    if (!fs.statSync(appDir).isDirectory()) {
        throw new UserError(`${appDir} is not a directory`);
    }

    const packageJson = path.join(appDir, "package.json");
    if (!fs.existsSync(packageJson)) {
        throw new UserError(`${packageJson} does not exist`);
    }

    const appJs = path.join(appDir, "app.js");
    const appTs = path.join(appDir, "app.ts");
    if (!fs.existsSync(appJs) && !fs.existsSync(appTs)) {
        throw new UserError(`Neither ${appJs} nor ${appTs} does not exist.`);
    }

    return appDir;
};

const availableBoardTypes = ["esp32"];
export const validateBoardType: Validator = (boardType: string): any => {
    if (!availableBoardTypes.includes(boardType)) {
        throw new UserError("Invalid board type.");
    }

    return require(`../boards/${boardType}`);
};

const availableCloudTypes = ["firebase"];
export const validateCloudType: Validator = (cloudType: string): any => {
    if (!availableCloudTypes.includes(cloudType)) {
        throw new UserError("Invalid cloud type.");
    }

    return require(`../clouds/${cloudType}`);
};


const deviceFilePatterns = [
    /tty\.usbserial-.+/,
    /ttyUSB[0-9]/,
];
export const validateDeviceFilePath: Validator = (deviceFile: string): any => {
    if (deviceFile.length > 0) {
        return deviceFile;
    }

    const candidates = fs.readdirSync("/dev").filter((file) => {
        return deviceFilePatterns.some((pat) => pat.exec(file) !== null);
    });

    if (candidates.length === 0) {
        throw new UserError("Failed to locate the device file.");
    }

    if (candidates.length > 1) {
        throw new UserError("Found multiple device files. Please specify the one in the command-line option.");
    }

    return path.join("/dev", candidates[0]);
}

export const APP_OPTS = [
    {
        name: "--app-dir <path>",
        desc: "The app directory.",
        default: process.cwd(),
        validator: validateAppDir,
    },
];

export const ADAPTER_OPTS = [
    {
        name: "--adapter <adapter>",
        desc: "The adapter type ('serial' or 'wifi').",
        default: "serial",
    },
]

export const BOARD_OPTS = [
    // TODO: get the board type from package.json
    {
        name: "--board <board>",
        desc: "The board type (only 'esp32' for now).",
        default: "esp32",
        validator: validateBoardType,
    },
];

export const DEVICE_FILE_OPTS = [
    {
        name: "--device <path>",
        desc: "The device file path.",
        default: "",
        validator: validateDeviceFilePath,
    },
    {
        name: "--baudrate <rate>",
        desc: "The baudrate.",
        default: 115200,
    },
];

export const BUILD_OPTS = [
    {
        name: "--heartbeat-interval <seconds>",
        desc: "The interval of the heartbeat periodically sent from a device.",
        default: 30,
    },
    ...APP_OPTS,
    ...ADAPTER_OPTS,
    ...BOARD_OPTS
];

export const WATCH_OPTS = [
    {
        name: "--watch",
        desc: "Watch files for changes and ru-run the command.",
        default: false,
    }
]

export const CLOUD_OPTS = [
    {
        name: "--cloud <cloud>",
        desc: "The cloud (only 'firebase' for now).",
        default: "firebase",
        validator: validateCloudType,
    },
    {
        name: "--firebase-project <name>",
        desc: "The Firebase project name.",
    },
]
