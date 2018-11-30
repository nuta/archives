import { api } from "../api";

export async function list(args: any, opts: any, logger: any) {
    for (const config of await api.getDeviceConfigs(opts.deviceName)) {
        logger.info(config);
    }
}

export async function set(args: any, opts: any, logger: any) {
    await api.setDeviceConfig(opts.deviceName, args.name, opts.type, args.value);
}

export async function delete_(args: any, opts: any, logger: any) {
    await api.deleteDeviceConfig(opts.deviceName, args.name);
}
