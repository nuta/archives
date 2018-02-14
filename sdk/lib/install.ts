import * as crypto from "crypto";
import * as fs from "fs";
import * as os from "os";
import * as path from "path";
const fetch = require("node-fetch");
const ipc = require("node-ipc");
const { quote } = require("shell-quote");
const sudo = require("sudo-prompt");
import { api } from "./api";
import { getDriveSize } from "./drive";
import {
    createFile, generateRandomString,
    generateTempPath, getenv, shasum
} from "./helpers";
import { FatalError, APIError } from "./types";

export type ProgressCallback = (stage: string, meta?: any) => void

function replaceBuffer(buf: Buffer, value: string, id: string): Buffer {
    const needle = `_____REPLACE_ME_MAKESTACK_CONFIG_${id}_____`;

    const index = buf.indexOf(Buffer.from(needle));
    if (index === -1) {
        throw new FatalError(`replaceBuffer: failed to replace ${id}`);
    }

    const paddedValue = Buffer.alloc(needle.length, " ");
    const valueBuf = Buffer.from(value);
    valueBuf.copy(paddedValue);
    paddedValue.copy(buf, index);
    return buf;
}

async function registerOrGetDevice(name: string, type: string, app: string): Promise<any> {
    let device;
    try {
        device = await api.registerDevice(name, type, app);
    } catch (e) {
        if (e instanceof APIError && e.response.errors[0] === 'Name has already been taken') {
            // There is already a device with same name.
            try {
                device = await api.getDevice(name);
            } catch (e) {
                throw new FatalError("Failed to fetch the device metadata.");
            }
        } else {
            throw e;
        }
    }

    return device;
}

function getLatestOSRelease(osType: string, deviceType: string):
    Promise<{ version: string, url: string, shasum: string }>
{
    return new Promise((resolve, reject) => {
        api.getOSReleases().then(({ releases }) => {
            const version = Object.keys(releases).pop();
            if (!version) {
                throw new Error('no os releases');
            }

            const asset = releases[version][osType].assets[deviceType];
            resolve({
                version,
                url: asset.url,
                shasum: asset.shasum
            });
        }).catch(reject);
    });
}

async function downloadDiskImage(osType: string, deviceType: string) {
    const { version, url: osImageURL, shasum: imageShasum } =
        await getLatestOSRelease(osType, deviceType);
    const basename = path.basename(osImageURL);
    const originalImage = path.join(getenv('HOME'), `.makestack/caches/${basename}`);
    if (!fs.existsSync(originalImage) || shasum(originalImage) !== imageShasum) {
        createFile(originalImage, await (await fetch(osImageURL, { redirect: 'follow' })).buffer());
    }
    return originalImage;
}

function writeConfigToDiskIamge(args: {
    deviceType: string,
    originalImage: string,
    device: any,
    adapter: string,
    wifiSSID?: string,
    wifiPassword?: string,
    wifiCountry?: string
}) {
    const {
        deviceType,
        originalImage,
        device,
        adapter,
        wifiSSID = '',
        wifiPassword = '',
        wifiCountry = ''
    } = args

    const imagePath = generateTempPath('installer', '.img');
    const wifiPsk = crypto.pbkdf2Sync(wifiPassword, wifiSSID, 4096, 256, "sha1").toString('hex').substring(0, 64);

    // TODO: What if the image is large?
    let image = fs.readFileSync(originalImage);
    image = replaceBuffer(image, deviceType, "DEVICE_TYPE");
    image = replaceBuffer(image, device.device_id, "DEVICE_ID");
    image = replaceBuffer(image, device.device_secret, "DEVICE_SECRET");
    image = replaceBuffer(image, api.serverURL, "SERVER_URL_abcdefghijklmnopqrstuvwxyz1234567890");
    image = replaceBuffer(image, adapter, "NETWORK_ADAPTER");
    image = replaceBuffer(image, wifiSSID, "WIFI_SSID");
    image = replaceBuffer(image, wifiPsk, "WIFI_PSK");
    image = replaceBuffer(image, wifiCountry, "WIFI_COUNTRY");
    fs.writeFileSync(imagePath, image);

    return imagePath;
}

function prepareFlashCommand(flashCommand: string, ipcPath: string, drive: string,
    driveSize: number, imagePath: string) {

    let prefix = "env ";
    const env: { [name: string]: string } = {
        DRIVE: drive,
        IMAGE_WRITER: "y",
        DRIVE_SIZE: driveSize.toString(),
        IMAGE_PATH: imagePath,
        IPC_PATH: ipcPath,
        ELECTRON_RUN_AS_NODE: "1",
    };

    for (const name in env) {
        prefix += `${name}=${quote([env[name]])} `;
    }

    return prefix + quote(flashCommand);
}

function flash(flashCommand: string, drive: string, driveSize: number,
    imagePath: string, progressCallback: ProgressCallback) {

    return new Promise((resolve, reject) => {
        const ipcPath = path.join(os.tmpdir(),
            "makestack-installer" +
            generateRandomString(32));

        ipc.config.logger = () => { };
        ipc.serve(ipcPath, () => {
            ipc.server.on("progress", (data: any) => {
                progressCallback("flashing", JSON.parse(data));
            });
        });
        ipc.server.start();

        const command = prepareFlashCommand(
            flashCommand, ipcPath, drive, driveSize, imagePath
        );
        const options = { name: "MakeStack Installer" };
        sudo.exec(command, options, (error: Error, stdout: any, stderr: any) => {
            if (error) {
                reject(error);
            }

            ipc.server.stop();
            resolve();
        });
    });
}

export async function install(args: {
    deviceName: string,
    deviceType: string,
    osType: string,
    app: string,
    adapter: string,
    wifiSSID: string,
    wifiPassword: string,
    wifiCountry: string,
    drive: string,
    flashCommand: string,
    diskImagePath?: string
}, progressCallback: ProgressCallback) {

    const {
        deviceName, deviceType, osType, adapter, wifiSSID, wifiPassword, wifiCountry,
        drive, flashCommand, diskImagePath, app
    } = args;

    progressCallback("look-for-drive");
    const driveSize = await getDriveSize(drive);
    progressCallback("register");
    const device = await registerOrGetDevice(deviceName, deviceType, app);
    progressCallback("download");

    let originalImage
    if (diskImagePath) {
        originalImage = diskImagePath;
    } else {
        originalImage = await downloadDiskImage(osType, deviceType);
    }

    progressCallback("config");
    const imagePath = writeConfigToDiskIamge({
        deviceType, originalImage, device, adapter,
        wifiSSID, wifiPassword, wifiCountry
    })
    progressCallback('flash')
    await flash(flashCommand, drive, driveSize, imagePath, progressCallback)
    progressCallback('success')
}
