import { api } from "../api";
import { loadPackageJson } from "../appdir";

export async function main(args: any, opts: any, logger: any) {
    const appName = loadPackageJson(opts.appDir).name;

    await api.updateDevice(args.deviceName, {
        app: appName,
    });
}
