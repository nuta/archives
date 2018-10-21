import "mocha";
import * as assert from "assert";
import * as path from "path";
import * as fs from "fs-extra";
import { main } from "../../lib";
import { logger } from "../../lib/logger";
import { tmpdir } from "../helpers";

describe("new command", function() {
    beforeEach(function() {
        logger.disableStdout();
        this.tmpdir = tmpdir.enter();
    })

    afterEach(function() {
        this.tmpdir.restore();
        logger.enableStdout();
    })

    it("creates a new file", function() {
        const appDir = this.tmpdir.tmpdir;
        main.run(['new', appDir]);
        assert.ok(fs.existsSync(path.join(appDir, 'device.cc')));
    })
})
