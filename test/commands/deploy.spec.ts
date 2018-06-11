import "mocha";
import * as sinon from "sinon";
import * as path from "path";
import * as fs from "fs-extra";
import * as childProcess from "child_process";
import { expect } from "chai";
import { main } from "../..";

describe("deploy command", function() {
    beforeEach(async function() {
        const spawnSync = childProcess.spawnSync;
        sinon.stub(childProcess, "spawnSync")
            .callsFake((cmd, args, opts) => {
                if (cmd === "firebase") {
                    return spawnSync("echo", ["-n"]);
                } else {
                    return spawnSync(cmd, args, opts);
                }
            });

        this.cwd = process.cwd()
        this.appDir = "test-app"
        fs.removeSync(this.appDir)
        await main.run(['new', this.appDir])
    })

    afterEach(function() {
        fs.removeSync(this.appDir)
    })

    it("deploys successfully", async function() {
        await main.run([
            'deploy', '--app-dir', this.appDir,
            '--firebase-project', 'makestack-hello-world',
            '--platform', 'firebase'
        ]);
    })
})
