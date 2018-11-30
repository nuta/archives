import "mocha";
import * as sinon from "sinon";
import * as assert from "assert";
import { main } from "../../lib";
import { appdir } from "../helpers";
import { getSdkInstance } from "../../lib/platform";
import { logger } from "../../lib/logger";

describe("command command", function() {
    beforeEach(async function() {
        logger.disableStdout();
        const sdk = getSdkInstance("firebase");
        this.command = sinon.stub(sdk, "command").returns(undefined);
        this.appdir = await appdir.enter();
    })

    afterEach(function() {
        this.command.restore();
        this.appdir.restore();
        logger.enableStdout();
    })

    it("calls sdk.command", async function() {
        const deviceName = "my-esp32";
        const cmdName = "light";
        const cmdArg = "on";

        await main.run([
            "command","--env", "production",
            "--app-dir", this.appdir.tmpdir,
            "--device", deviceName,
            cmdName, cmdArg
        ]);

        assert.ok(this.command.withArgs(deviceName, cmdName, cmdArg));
    })
})
