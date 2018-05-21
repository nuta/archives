import { api } from "../api";
import { loadPackageJson } from "../appdir";

export async function list(args: any, opts: any, logger: any) {
    const appName = loadPackageJson(opts.appDir).name;

    for (const config of await api.getAppConfigs(appName)) {
        logger.info(config);
    }
}

export async function set(args: any, opts: any, logger: any) {
    const appName = loadPackageJson(opts.appDir).name;
    await api.setAppConfig(appName, args.name, opts.type, args.value);
}

export async function delete_(args: any, opts: any, logger: any) {
    const appName = loadPackageJson(opts.appDir).name;
    await api.deleteAppConfig(appName, args.name);
}
