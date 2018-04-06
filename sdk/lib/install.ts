import * as crypto from "crypto";
import * as fs from "fs";
import * as os from "os";
import * as path from "path";
const fetch = require("node-fetch");
const ipc = require("node-ipc");
const { quote } = require("shell-quote");
const sudo = require("sudo-prompt");
const { esptool } = require("esptoolpy");
import { api } from "./api";
import { getDriveSize } from "./drive";
import {
    createFile, generateRandomString,
    generateTempPath, getenv, shasum
} from "./helpers";
import { FatalError, APIError } from "./types";

export type ProgressCallback = (stage: string, meta?: any) => void

function checkDriveExistence(drivePath: string) {
    return fs.existsSync(drivePath)
}

function replaceBuffer(buf: Buffer, value: string, id: string, fill: number): Buffer {
    const needle = `_____REPLACE_ME_MAKESTACK_CONFIG_${id}_____`;

    const index = buf.indexOf(Buffer.from(needle));
    if (index === -1) {
        throw new FatalError(`replaceBuffer: failed to replace ${id}`);
    }

    const paddedValue = Buffer.alloc(needle.length, fill);
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

function getLatestOSRelease(deviceType: string):
    Promise<{ version: string, url: string, shasum: string }>
{
    return new Promise((resolve, reject) => {
        api.getOSReleases().then(({ releases }) => {
            const version = Object.keys(releases).pop();
            if (!version) {
                throw new Error('no os releases');
            }

            const asset = releases[version].assets[deviceType];
            resolve({
                version,
                url: asset.url,
                shasum: asset.shasum
            });
        }).catch(reject);
    });
}

async function downloadDiskImage(deviceType: string) {
    const { version, url: osImageURL, shasum: imageShasum } =
        await getLatestOSRelease(deviceType);
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
    wifiCountry?: string,
    url: string
}) {
    const {
        deviceType,
        originalImage,
        device,
        adapter,
        wifiSSID = '',
        wifiPassword = '',
        wifiCountry = '',
        url
    } = args

    const imagePath = generateTempPath('installer', '.img');
    let wifiPsk, fill
    if (deviceType === 'esp32') {
        wifiPsk = wifiPassword
        fill = 0x00
    } else {
        wifiPsk = crypto.pbkdf2Sync(wifiPassword, wifiSSID, 4096, 256, "sha1")
        .toString('hex').substring(0, 64);
        fill = 0x20 // white space
    }

    // TODO: What if the image is large?
    let image = fs.readFileSync(originalImage);
    image = replaceBuffer(image, deviceType, "DEVICE_TYPE", fill);
    image = replaceBuffer(image, device.device_id, "DEVICE_ID", fill);
    image = replaceBuffer(image, device.device_secret, "DEVICE_SECRET", fill);
    image = replaceBuffer(image, url, "SERVER_URL_abcdefghijklmnopqrstuvwxyz1234567890", fill);
    image = replaceBuffer(image, adapter, "NETWORK_ADAPTER", fill);
    image = replaceBuffer(image, wifiSSID, "WIFI_SSID", fill);
    image = replaceBuffer(image, wifiPsk, "WIFI_PSK", fill);
    image = replaceBuffer(image, wifiCountry, "WIFI_COUNTRY", fill);
    fs.writeFileSync(imagePath, image);

    return imagePath;
}

function prepareFlashCommand(deviceType: string, flashCommand: string,
    ipcPath: string, drive: string,
    driveSize: number, imagePath: string) {

    let prefix = "env ";
    const env: { [name: string]: string } = {
        DEVICE_TYPE: deviceType,
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

function flash(deviceType: string, flashCommand: string, drive: string, driveSize: number,
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
            deviceType, flashCommand, ipcPath, drive, driveSize, imagePath
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

function prepareEsp32Image(imagePath: string) {
    return new Promise((resolve, reject) => {
        const tmpImagePath = imagePath + '.tmp';
        fs.copyFileSync(imagePath, tmpImagePath);

        const cp = esptool([
            '--chip', 'esp32',
            'elf2image',
            '--flash_mode', 'dio',
            '--flash_freq', '40m',
            '--flash_size', '2MB',
            '-o', imagePath,
            tmpImagePath
        ]);

        cp.stderr.on('data', (error: Buffer) => console.log(error.toString()));

        cp.on('close', (code: number) => {
            fs.unlinkSync(tmpImagePath);

            if (code === 0)
                resolve()
            else
                reject(new Error(`esptool elf2image returned ${code}`))
        });
    });
}

export async function install(args: {
    deviceName: string,
    deviceType: string,
    app: string,
    adapter: string,
    wifiSSID: string,
    wifiPassword: string,
    wifiCountry: string,
    drive: string,
    flashCommand: string,
    diskImagePath?: string,
    url: string
}, progressCallback: ProgressCallback) {

    const {
        deviceName, deviceType, adapter, wifiSSID, wifiPassword, wifiCountry,
        drive, flashCommand, diskImagePath, app, url
    } = args;

    progressCallback("look-for-drive");
    checkDriveExistence(drive)
    const driveSize = (deviceType === 'esp32') ? 0 : await getDriveSize(drive);

    progressCallback("register");
    const device = await registerOrGetDevice(deviceName, deviceType, app);
    progressCallback("download");

    let originalImage
    if (diskImagePath) {
        originalImage = diskImagePath;
    } else {
        originalImage = await downloadDiskImage(deviceType);
    }

    progressCallback("config");
    const imagePath = writeConfigToDiskIamge({
        deviceType, originalImage, device, adapter,
        wifiSSID, wifiPassword, wifiCountry, url
    })

    if (deviceType === 'esp32') {
        await prepareEsp32Image(imagePath)
    }

    progressCallback('flash')
    await flash(deviceType, flashCommand, drive, driveSize, imagePath, progressCallback)
    progressCallback('success')
}
