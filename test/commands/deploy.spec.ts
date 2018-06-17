import "mocha";
import * as sinon from "sinon";
import * as path from "path";
import { main } from "../../lib";
import { board } from "../../lib/boards";
import * as helpers from "../../lib/helpers";
import { appdir } from "../helpers";
import { getSdkInstance } from "../../lib/platform";
import { logger } from "../../lib/logger";

describe("deploy command", function() {
    beforeEach(async function() {
        logger.disableStdout();

        this.downloadRepo = sinon.stub(helpers, "downloadRepo").callsFake((appDir) => {
            return path.resolve(__dirname, "../..");
        })

        const sdk = getSdkInstance("firebase");
        this.build = sinon.stub(board, "build").returns(Promise.resolve());
        this.deploy = sinon.stub(sdk, "deploy").returns(Promise.resolve());
        this.appdir = await appdir.enter();
    })

    afterEach(function() {
        this.downloadRepo.restore();
        this.build.restore();
        this.deploy.restore();
        this.appdir.restore();
        logger.enableStdout();
    })

    it("deploys successfully", async function() {
        await main.run(['deploy', '--app-dir', this.appdir.tmpdir]);
    })
})
