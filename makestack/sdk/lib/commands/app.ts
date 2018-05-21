import { api } from "../api";

export async function list(args: any, opts: any, logger: any) {
    for (const app of await api.getApps()) {
        logger.info(app);
    }
}

export async function create(args: any, opts: any, logger: any) {
    await api.createApp(args.name, opts.api);
}

export async function delete_(args: any, opts: any, logger: any) {
    await api.deleteApp(args.name);
}
