import { api } from "../api";

export async function list(args: any, opts: any, logger: any) {
    for (const device of await api.getDevices()) {
        logger.info(device);
    }
}

export async function delete_(args: any, opts: any, logger: any) {
    await api.deleteDevice(args.name);
}
