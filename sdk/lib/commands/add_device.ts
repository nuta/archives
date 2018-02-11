import { api } from "../api";
import { loadPackageJson } from "../appdir";

export async function main(args: any, opts: any, logger: any) {
    await api.updateDevice(args.name, {
        app: opts.app
    });
}
