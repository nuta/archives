const fs = require("fs");
const imageWrite = require("etcher-image-write");
const mountutils = require("mountutils");
const ipc = require("node-ipc");

/*
*  Be careful! This command will be executed in root!!!
*/

function connectIPC(ipcPath) {
    return new Promise((resolve, reject) => {
        ipc.config.retry = 5;
        ipc.config.maxRetries = 3;
        ipc.config.logger = () => { };
        ipc.connectTo("server", ipcPath, () => {
            ipc.of.server.on("connect", () => resolve(ipc.of.server));
        });
    });
}

function umount(drive) {
    return new Promise((resolve, reject) => {
        mountutils.unmountDisk(drive, (error) => {
            if (error) {
                reject(error);
            } else {
                resolve();
            }
        });
    });
}

export async function imageWriter() {
    const ipcPath = process.env.IPC_PATH;
    const drive = process.env.DRIVE;
    const driveSize = parseInt(process.env.DRIVE_SIZE);
    const imagePath = process.env.IMAGE_PATH;

    if (ipcPath === undefined || drive === undefined ||
        driveSize === undefined || imagePath === undefined) {
            throw new Error("specify `IPC_PATH', `DRIVE_PATH', `DRIVE_SIZE' and `IMAGE_PATH'");
        }

        console.log("image-writer:",
        `drive=${drive}, drive_size=${driveSize}, image_path=${imagePath}`);

        console.log("image-writer:", `connecting to ${ipcPath}`);
        const server: any = await connectIPC(ipcPath);

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

        writer.on("progress", (state) => {
            server.emit("progress", JSON.stringify(state));
            console.log("image-writer:", `${state.type} ${state.percentage}%`);
        });

        writer.on("done", (results) => {
            server.emit("success", JSON.stringify(results));
            console.log("image-writer:", "done");
            ipc.disconnect("server");
        });

        writer.on("error", (error) => {
            server.emit("error", error);
            console.error("image-writer:", "error:", error);
            ipc.disconnect("server");
        });
    }
