import * as inquirer from "inquirer";
import { API } from "../api";
import { saveDeviceConfig } from "../config";

export async function main(args: any, opts: any, logger: any) {
    const answers = await inquirer.prompt([
        { message: "User Password", name: "password", type: "password" }
    ]);

    // Log in without saving credentials to a file.
    let cred: any = null;
    const api = new API(() => cred, c => cred = c);
    await api.login(opts.server, opts.username, answers.password)

    const device = await api.registerDevice(opts.name, "sdk", opts.app)

    saveDeviceConfig(Object.assign({}, device, { serverURL: opts.server }))
}

