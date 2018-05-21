const fs = require("fs");
const os = require("os");
const path = require("path");
const imageWrite = require("etcher-image-write");
const mountutils = require("mountutils");
const ipc = require("node-ipc");
const { esptool, ARDUINO_DIR } = require("esptoolpy");
import { getenv } from './helpers';

/*
*  Be careful! This command will be executed in root!!!
*/

function connectIPC(ipcPath: string) {
    return new Promise((resolve, reject) => {
        ipc.config.retry = 5;
        ipc.config.maxRetries = 3;
        ipc.config.logger = () => { };
        ipc.connectTo("server", ipcPath, () => {
            ipc.of.server.on("connect", () => resolve(ipc.of.server));
        });
    });
}

function umount(drive: string) {
    return new Promise((resolve, reject) => {
        mountutils.unmountDisk(drive, (error: Error) => {
            if (error) {
                reject(error);
            } else {
                resolve();
            }
        });
    });
}

export interface WriterArgs {
    drive: string;
    driveSize?: number;
    server: any;
    imagePath: string;
}

async function flashByDd({ drive, driveSize, server, imagePath }: WriterArgs) {
    console.log("image-writer:", "unmounting");
    await umount(drive);

    console.log("image-writer:", "writing");
    const writer = imageWrite.write({
        fd: fs.openSync(drive, "rs+"),
        device: drive,
        size: driveSize,
    }, {
        stream: fs.createReadStream(imagePath),
        size: fs.statSync(imagePath).size,
    }, {
        check: true,
    });

    writer.on("progress", (state: any) => {
        server.emit("progress", JSON.stringify(state));
        console.log("image-writer:", `${state.type} ${state.percentage}%`);
    });

    writer.on("done", (results: any[]) => {
        server.emit("success", JSON.stringify(results));
        console.log("image-writer:", "done");
        ipc.disconnect("server");
    });

    writer.on("error", (error: Error) => {
        server.emit("error", error);
        console.error("image-writer:", "error:", error);
        ipc.disconnect("server");
    });
}

async function flashByEsptool({ drive, server, imagePath }: WriterArgs) {
    console.log("image-writer:", "writing");
    const cp = esptool([
        '--chip', 'esp32',
        '--port', drive,
        '--baud', '115200',
        '--before', 'default_reset',
        '--after', 'hard_reset',
        'write_flash',
        '-z',
        '--flash_mode', 'dio', '--flash_freq', '40m', '--flash_size', 'detect',
        '0x1000', path.resolve(ARDUINO_DIR, 'tools/sdk/bin/bootloader_dio_40m.bin'),
        '0x8000', path.resolve(ARDUINO_DIR, 'tools/partitions/default.bin'),
        '0xe000', path.resolve(ARDUINO_DIR, 'tools/partitions/boot_app0.bin'),
        '0x10000', path.resolve(imagePath)
    ]);

    cp.stdout.on('data', (s: Buffer) => {
        const str = s.toString()
        const progressRegex = /Writing at 0x[0-9a-f]+\.{3} \((\d+ %)\)/
        for (const line of str.split("\n")) {
            const match = progressRegex.exec(line)
            if (match !== null) {
                server.emit("progress", JSON.stringify({
                    type: 'write',
                    percentage: parseInt(match[1])
                }))
            }
        }
    })

    cp.stderr.on('data', (s: Buffer) => console.error(s.toString()))

    cp.on('close', (code: number) => {
        if (code === 0) {
            server.emit('success', JSON.stringify({}))
            console.log("image-writer:", "done");
        } else {
            server.emit('error', JSON.stringify({}))
            console.error("image-writer:", `error: ${code}`);
        }

        ipc.disconnect("server");
    })
}

export async function imageWriter() {
    const deviceType = getenv('DEVICE_TYPE');
    const ipcPath = getenv('IPC_PATH');
    const drive = getenv('DRIVE');
    const driveSize = parseInt(getenv('DRIVE_SIZE'));
    const imagePath = getenv('IMAGE_PATH');

    console.log(`image-writer: drive=${drive}, drive_size=${driveSize}, image_path=${imagePath}`);

    console.log("image-writer:", `connecting to ${ipcPath}`);
    const server: any = await connectIPC(ipcPath);

    switch (deviceType) {
        case 'raspberrypi3':
            await flashByDd({ drive, driveSize, server, imagePath });
            break;
        case 'esp32':
            await flashByEsptool({ drive, server, imagePath });
            break;
    }
}
