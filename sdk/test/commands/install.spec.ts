import { expect } from "chai";
import * as child_process from "child_process";
import * as fs from "fs";
import * as os from "os";
import * as path from "path";
import * as nock from "nock";
import * as sinon from "sinon";
import * as mockfs from "mock-fs";
import {
    CONFIG_FILES, login, createApiRequest
} from "../helpers";

const installCommand = require("../../dist/commands/install").main

describe("install command", function() {
    beforeEach(function() {
        const drives = [
            {
                displayName: "SD Card Reader",
                description: "",
                size: 8 * 1024 * 1024 * 1024,
                device: "/dev/disk2s1",
                system: false
            }
        ]
        this.drivelist = sinon.stub(require("drivelist"), "list")
            .callsFake(callback => callback(null, drives))

        mockfs(CONFIG_FILES);
        login();

        this.sudo = sinon.stub(require("sudo-prompt"), "exec")
            .callsFake((command, options, callback) => callback())

        // XXX: We cannot create a stub of node-ipc. We have to replace node-ipc
        //      with child_process.fork().
        //
        // Refer: https://github.com/RIAEvangelist/node-ipc/blob/1d8fae31ec0f21cd08fcdc5e23f7b756d4633b55/services/IPC.js#L35
        //
        /* this.ipc = sinon.stub(require("node-ipc"), "serve"); */

        this.osImageURL = 'https://makestack-s3/raspberrypi3.img'
        this.device = {
            name: "my-rpi2",
            device_type: "raspberrpi3"
        }

        this.deviceRegistrationRequest = createApiRequest()
            .post("/devices", { device: this.device })
            .reply(200, {
                device_id: 'h9pOUCIgo87gcIguyKCJgyuog6guas890y89svblqf2bjkl1',
                device_secret: 'joA09uhCp98y98i7ci9ho8cHIASChl67aCSkjasckjasjwka'
            })

        this.fetchOsReleasesRequest = createApiRequest()
            .get("/os/releases")
            .reply(200, {
                releases: {
                    'v20.2.3': {
                        linux: {
                            assets: {
                                raspberrpi3: {
                                    url: this.osImageURL
                                }
                            }
                        }
                    }
                }
            })

            this.osImageContent = `
# REPLACE_ME placeholders are replaced by MakeStack Installer.
export MAKESTACK_OS_VERSION="_____REPLACE_ME_MAKESTACK_CONFIG_OS_VERSION_____"
export MAKESTACK_DEVICE_TYPE="_____REPLACE_ME_MAKESTACK_CONFIG_DEVICE_TYPE_____"
export MAKESTACK_SERVER_URL="_____REPLACE_ME_MAKESTACK_CONFIG_SERVER_URL_abcdefghijklmnopqrstuvwxyz1234567890_____"
export MAKESTACK_DEVICE_ID="_____REPLACE_ME_MAKESTACK_CONFIG_DEVICE_ID_____"
export MAKESTACK_DEVICE_SECRET="_____REPLACE_ME_MAKESTACK_CONFIG_DEVICE_SECRET_____"
export NETWORK_ADAPTER="_____REPLACE_ME_MAKESTACK_CONFIG_NETWORK_ADAPTER_____"
export WIFI_SSID="_____REPLACE_ME_MAKESTACK_CONFIG_WIFI_SSID_____"
export WIFI_PSK="_____REPLACE_ME_MAKESTACK_CONFIG_WIFI_PSK_____"
export WIFI_COUNTRY="_____REPLACE_ME_MAKESTACK_CONFIG_WIFI_COUNTRY_____"

# The interval of heartbeating. (seconds)
export MAKESTACK_HEARTBEAT_INTERVAL=15
`

        this.osImageRequest = nock('https://makestack-s3')
            .get('/raspberrypi3.img')
            .reply(200, this.osImageContent)
    })

    afterEach(function () {
        mockfs.restore();
        nock.cleanAll();
        this.drivelist.restore();
        this.sudo.restore();
    })

    it("works", async function () {
        await installCommand({}, {
            name: this.device.name,
            type: this.device.device_type,
            os: "linux",
            adapter: "http",
            drive: "/dev/disk2s1",
            ignoreDuplication: false
        })
    })
})
