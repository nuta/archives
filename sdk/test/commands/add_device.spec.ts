import { expect } from 'chai';
import * as child_process from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import * as nock from 'nock';
import * as sinon from 'sinon';
import * as mockfs from 'mock-fs';
import {
    CONFIG_FILES, login, createApp, createApiRequest, createZip
} from '../helpers';

const addDeviceCommand = require('../../dist/commands/add_device').main

describe('add-device command', function() {
    beforeEach(function () {
        mockfs(Object.assign({}, CONFIG_FILES));
        this.appName = 'hello-world';
        this.appDir = createApp(this.appName);
        login();
    })

    afterEach(function () {
        mockfs.restore()
        nock.cleanAll()
    })

    it('invokes PATCH /devices/:name', async function() {
        const deviceName = 'my-raspi2'
        const request = createApiRequest()
            .patch(`/devices/${deviceName}`, { device: { app: this.appName } })
            .reply(200, {})

        await addDeviceCommand({ deviceName }, { app: this.appName, appDir: this.appDir });
        expect(request.isDone()).to.be.true;
    })
})
