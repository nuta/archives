import { api } from "../api";

export async function list(args, opts, logger) {
    for (const device of await api.getDevices()) {
        logger.info(device);
    }
}

export async function delete_(args, opts, logger) {
    await api.deleteDevice(args.name);
}
