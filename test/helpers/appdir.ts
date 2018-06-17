import * as tmpdir from "./tmpdir";
import { main } from "../../lib";
import { scaffold } from "../../lib/scaffold";


export async function enter(): Promise<tmpdir.TmpDir> {
    const ctx = tmpdir.enter();
    scaffold({ name: "test-app", dir: ".", frontend: "static" });
    return ctx;
}
