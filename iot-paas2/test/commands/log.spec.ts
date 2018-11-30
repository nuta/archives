import "mocha";
import * as sinon from "sinon";
import * as assert from "assert";
import { main } from "../../lib";
import { appdir } from "../helpers";
import { getSdkInstance } from "../../lib/platform";
import { logger } from "../../lib/logger";

describe("log command", function() {
    beforeEach(async function() {
        logger.disableStdout();
        const sdk = getSdkInstance("firebase");
        this.viewLog = sinon.stub(sdk, "viewLog").returns(undefined);
        this.appdir = await appdir.enter();
    })

    afterEach(function() {
        this.viewLog.restore();
        this.appdir.restore();
        logger.enableStdout();
    })

    it("calls sdk.viewLog", async function() {
        await main.run(['log', '--env', "production", '--app-dir', this.appdir.tmpdir]);
        assert.ok(this.viewLog.calledOnce);
    })
})
