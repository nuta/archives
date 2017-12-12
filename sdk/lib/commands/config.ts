import { api } from "../api";
import { loadAppYAML } from "../appdir";

export async function list(args: any, opts: any, logger: any) {
    const appName = loadAppYAML(opts.appDir).name;

    for (const config of await api.getAppConfigs(appName)) {
        logger.info(config);
    }
}

export async function set(args: any, opts: any, logger: any) {
    const appName = loadAppYAML(opts.appDir).name;
    await api.setAppConfig(appName, args.name, opts.type, args.value);
}

export async function delete_(args: any, opts: any, logger: any) {
    const appName = loadAppYAML(opts.appDir).name;
    await api.deleteAppConfig(appName, args.name);
}
