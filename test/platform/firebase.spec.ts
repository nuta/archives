import "mocha";
import * as fs from "fs-extra";
import * as path from "path";
import * as childProcess from "child_process";
import * as sinon from "sinon";
import * as assert from "assert";
import { main } from "../../lib";
import { board } from "../../lib/boards";
import * as helpers from "../../lib/helpers";
import { appdir } from "../helpers";
import { logger } from "../../lib/logger";
import { getSdkInstance } from "../../lib/platform";

describe("Firebase platform support", function() {
    beforeEach(async function() {
        logger.disableStdout();

        this.downloadRepo = sinon.stub(helpers, "downloadRepo").callsFake((appDir) => {
            return path.resolve(__dirname, "../..");
        })


        const realSpawn = childProcess.spawn;
        this.spawn = sinon.stub(childProcess, "spawn");
        this.spawn.callsFake(() => realSpawn("date"));

        const realSpawnSync = childProcess.spawnSync;
        this.spawnSync = sinon.stub(childProcess, "spawnSync");
        this.spawnSync.callsFake(() => realSpawnSync("date"));
        this.spawnSync.withArgs("touch").callsFake(() => realSpawnSync("date"));

        this.config = {
            production: {
                platform: "firebase",
                firebaseProject: "makestack-test-project"
            }
        };
        this.appdir = await appdir.enter(this.config);
        this.platform = await getSdkInstance("firebase");
        fs.writeFileSync("esp32.firmware", "this is an firmware");
    })

    afterEach(function() {
        this.downloadRepo.restore();
        this.spawn.restore();
        this.spawnSync.restore();
        this.appdir.restore();
        logger.enableStdout();
    })

    it("deploys", async function() {
        await this.platform.deploy(this.appdir.tmpdir, this.config.production);
    })
})
