import "mocha";
import * as fs from "fs";
import * as path from "path";
import * as childProcess from "child_process";
import * as sinon from "sinon";
import * as assert from "assert";
import { main } from "../../lib";
import { board } from "../../lib/boards";
import * as helpers from "../../lib/helpers";
import { appdir } from "../helpers";
import { logger } from "../../lib/logger";

describe("dev command", function() {
    beforeEach(async function() {
        logger.disableStdout();
        this.fsWatchers = []
        const realFsWatch = fs.watch;
        this.fsWatch = sinon.stub(fs, "watch").callsFake((file, opts) => {
            this.fsWatchers.push(realFsWatch(file, opts));
        })

        this.build = sinon.stub(board, "build").returns(Promise.resolve());
        this.downloadRepo = sinon.stub(helpers, "downloadRepo").callsFake((appDir) => {
            return path.resolve(__dirname, "../..");
        })

        const realFork = childProcess.fork;
        this.forks = [];
        this.fork = sinon.stub(childProcess, "fork").callsFake(() => {
            const cp = realFork("echo", ["-n"]);
            this.forks.push(cp);
            return cp;
        })

        this.appdir = await appdir.enter();
    })

    afterEach(function() {
        this.downloadRepo.restore();
        this.build.restore();
        this.fork.restore();
        this.appdir.restore();
        logger.enableStdout();
        this.fsWatchers.map((watcher: fs.FSWatcher) => watcher.close());
        this.forks.map((cp: childProcess.ChildProcess) => cp.kill());
    })

    it("runs without errors", async function() {
        await main.run(['dev', '--app-dir', this.appdir.tmpdir]);
    })
})
