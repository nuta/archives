import * as crypto from "crypto";
import * as fs from "fs";
import * as fetch from "node-fetch";
import * as ipc from "node-ipc";
import * as os from "os";
import * as path from "path";
import { quote } from "shell-quote";
import * as sudo from "sudo-prompt";
import { api } from "./api";
import { getDriveSize } from "./drive";
import {
    createFile, generateRandomString,
    generateTempPath,
} from "./helpers";

function replaceBuffer(buf, value, id) {
    const needle = `_____REPLACE_ME_MAKESTACK_CONFIG_${id}_____`;

    const index = buf.indexOf(Buffer.from(needle));
    if (index === -1) {
        throw new Error(`replaceBuffer: failed to replace ${id}`);
    }

    const paddedValue = Buffer.alloc(needle.length, " ");
    const valueBuf = Buffer.from(value);
    valueBuf.copy(paddedValue);
    paddedValue.copy(buf, index);
    return buf;
}

async function registerOrGetDevice(name, type, ignoreDuplication) {
    let device;
    try {
        device = await api.registerDevice(name, type);
    } catch (e) {
        // FIXME: add an option to accept 4xx errors
        if (ignoreDuplication && e.message.includes("name: [ 'has already been taken' ]")) {
            // There is already a device with same name.
            device = await api.getDevice(name);
        } else {
            throw new Error("failed to register/fetch the device");
        }
    }

    return device;
}

function getLatestOSRelease(osType, deviceType): Promise<{ version: string, osImageURL: string }> {
    return new Promise((resolve, reject) => {
        api.getOSReleases().then(({ releases }) => {
            const version = Object.keys(releases).pop();
            const osImageURL = releases[version][osType].assets[deviceType].url;
            resolve({ version, osImageURL });
        }).catch(reject);
    });
}

async function downloadDiskImage(osType, deviceType) {
    const { version, osImageURL } = await getLatestOSRelease(osType, deviceType);
    const basename = path.basename(osImageURL);
    const orignalImage = path.join(process.env.HOME, `.makestack/caches/${basename}`);
    createFile(orignalImage, await (await fetch(osImageURL)).buffer());
    return [version, orignalImage];
}

function writeConfigToDiskIamge({ osVersion, deviceType, orignalImage, device, adapter, wifiSSID, wifiPassword, wifiCountry }) {
  const imagePath = generateTempPath();

  const wifiPsk = crypto.pbkdf2Sync(wifiPassword, wifiSSID, 4096, 256, "sha1").toString('hex').substring(0, 64);

  // TODO: What if the image is large?
  let image = fs.readFileSync(orignalImage);
  image = replaceBuffer(image, deviceType, "DEVICE_TYPE");
  image = replaceBuffer(image, osVersion, "OS_VERSION");
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

function prepareFlashCommand(flashCommand, ipcPath, drive, driveSize, imagePath) {
    let prefix = "env ";
    const env = {
        DRIVE: drive,
        IMAGE_WRITER: "y",
        DRIVE_SIZE: driveSize,
        IMAGE_PATH: imagePath,
        IPC_PATH: ipcPath,
        ELECTRON_RUN_AS_NODE: "1",
    };

    for (const name in env) {
        prefix += `${name}=${quote([env[name]])} `;
    }

    return prefix + quote(flashCommand);
}

function flash(flashCommand, drive, driveSize, imagePath, progress) {
    return new Promise((resolve, reject) => {
        const ipcPath = path.join(os.tmpdir(),
        "makestack-installer" + generateRandomString(32));

        ipc.config.logger = () => { };
        ipc.serve(ipcPath, () => {
            ipc.server.on("progress", (data) => {
                progress("flashing", JSON.parse(data));
            });
        });
        ipc.server.start();

        const command = prepareFlashCommand(flashCommand, ipcPath, drive, driveSize, imagePath);
        const options = { name: "MakeStack Installer" };
        sudo.exec(command, options, (error, stdout, stderr) => {
            if (error) { reject(error); }

            ipc.server.stop();
            resolve();
        });
    });
}

export async function install({
    deviceName, deviceType, osType, adapter, wifiSSID, wifiPassword, wifiCountry,
    drive, ignoreDuplication, flashCommand,
},                            progress) {

    progress("look-for-drive");
    const driveSize = await getDriveSize(drive);
    progress("register");
    const device = await registerOrGetDevice(deviceName, deviceType, ignoreDuplication);
    progress("download");
    const [osVersion, orignalImage] = await downloadDiskImage(osType, deviceType);
    progress("config");
    const imagePath = writeConfigToDiskIamge({
        osVersion, deviceType, orignalImage, device, adapter,
        wifiSSID, wifiPassword, wifiCountry
    })
    progress('flash')
    await flash(flashCommand, drive, driveSize, imagePath, progress)
    progress('success')
}
