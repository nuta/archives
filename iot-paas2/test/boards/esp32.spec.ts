import "mocha";
import * as sinon from "sinon";
import * as fs from "fs";
import * as path from "path";
import * as childProcess from "child_process";
import { main } from "../../lib";
import { board } from "../../lib/boards";
import * as helpers from "../../lib/helpers";
import { appdir } from "../helpers";
import { logger } from "../../lib/logger";
import { Esp32Board } from "../../lib/boards/esp32";

describe("esp32 board support", function() {
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

        this.appdir = await appdir.enter();
        this.board = new Esp32Board();
    })

    afterEach(function() {
        this.downloadRepo.restore();
        this.spawn.restore();
        this.spawnSync.restore();
        this.appdir.restore();
        logger.enableStdout();
    })

    it("builds an image without errors", async function() {
        this.board.build(
            false,
            "v0",
            path.resolve(__dirname, "../.."),
            this.appdir.tmpdir
        );
    })

    it("installs an image without errors", async function() {
        this.board.install(
            path.resolve(__dirname, "../.."),
            this.appdir.tmpdir,
            {
                deviceName: "my-esp32",
                serial: "/dev/ttyUSB0",
                serverUrl: "http://192.168.5.1:7878",
                adapter: "wifi",
                wifiSsid: "My AP",
                wifiPassword: "password is password",
            },
        );
    })
})
