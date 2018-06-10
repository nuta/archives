import "mocha";
import * as path from "path";
import * as fs from "fs-extra";
import * as mockfs from "mock-fs";
import { expect } from "chai";
import { main } from "../..";

describe("new command", function() {
    beforeEach(function() {
        this.appDir = "Development/hello"
        mockfs({
            [path.dirname(this.appDir)]: {}
        })
    })

    afterEach(function() {
        mockfs.restore()
    })

    it("creates a new file", function() {
        main.run(['new', this.appDir])
        expect(fs.existsSync(path.join(this.appDir, 'device.cc'))).to.equal(true);
    })
})
