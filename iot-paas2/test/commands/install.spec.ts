import "mocha";
import * as sinon from "sinon";
import * as assert from "assert";
import * as path from "path";
import { main } from "../../lib";
import { board } from "../../lib/boards";
import * as helpers from "../../lib/helpers";
import { appdir } from "../helpers";
import { logger } from "../../lib/logger";

describe("install command", function() {
    beforeEach(async function() {
        logger.disableStdout();

        this.repoDir = path.resolve(__dirname, "../..");
        this.downloadRepo = sinon.stub(helpers, "downloadRepo").callsFake((appDir) => {
            return this.repoDir;
        })

        this.build = sinon.stub(board, "build").returns(Promise.resolve());
        this.install = sinon.stub(board, "install").returns(Promise.resolve());
        this.appdir = await appdir.enter();
    })

    afterEach(function() {
        this.downloadRepo.restore();
        this.build.restore();
        this.install.restore();
        this.appdir.restore();
        logger.enableStdout();
    })

    it("calls board.install", async function() {
        await main.run([
            "install",
            "--app-dir", this.appdir.tmpdir,
            "--device-name", "my-esp32",
            "--adapter", "wifi",
            "--wifi-ssid", "my ap",
            "--wifi-password", "password is password",
            "--server-url", "http://192.168.1.102:7878",
            "--serial", "/dev/ttyUSB0",
        ]);
        assert.ok(this.install.calledOnce);
    })
})
