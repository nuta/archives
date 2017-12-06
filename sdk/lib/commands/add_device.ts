import { api } from "../api";
import { loadAppYAML } from "../appdir";

export async function main(args: any, opts: any, logger: any) {
    const appName = loadAppYAML(opts.appDir).name;

    await api.updateDevice(args.deviceName, {
        app: appName,
    });
}
