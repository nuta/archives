import * as fs from "fs-extra";
import * as tmpdir from "./tmpdir";
import { main } from "../../lib";
import { scaffold } from "../../lib/scaffold";


export async function enter(config?: any): Promise<tmpdir.TmpDir> {
    const ctx = tmpdir.enter();
    scaffold({ name: "test-app", dir: ".", frontend: "static" });

    const packageJson = fs.readJsonSync("package.json");
    Object.assign(packageJson["makestack"], config);
    fs.writeJsonSync("package.json", packageJson);

    return ctx;
}
