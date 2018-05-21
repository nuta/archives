import { } from 'mocha';
import { expect } from 'chai';
import * as child_process from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import * as nock from 'nock';
import * as sinon from 'sinon';
import * as mockfs from 'mock-fs';
import { CONFIG_FILES, login, createApiRequest } from '../helpers';

const {
    set: setCommand, list: listCommand, delete_: deleteCommand
} = require('../../dist/commands/device_config')

describe('device-config * commands', function() {
    beforeEach(function () {
        this.deviceName = 'my-rpi3'
        mockfs(Object.assign({}, CONFIG_FILES));
        login();
    })

    afterEach(function () {
        mockfs.restore()
        nock.cleanAll()
    })

    describe('device-config set command', function () {
        it('invokes PUT /devices/:app/configs/:name', async function () {
            const name = 'message'
            const dataType = 'string'
            const value = 'hello!'
            const request = createApiRequest()
                .put(`/devices/${this.deviceName}/configs/${name}`,
                    { config: { value, data_type: dataType } })
                .reply(200, {})

            await setCommand({ name, value }, { deviceName: this.deviceName, type: dataType });
                expect(request.isDone()).to.be.true;
        })
    })

    describe('device-config create command', function () {
        it('invokes POST /devices/:app/configs/:name', async function () {
            const name = 'message'
            const request = createApiRequest()
                .delete(`/devices/${this.deviceName}/configs/${name}`)
                .reply(200, {})

            await deleteCommand({ name }, { deviceName: this.deviceName });
            expect(request.isDone()).to.be.true;
        })
    })

    describe('device-config list command', function () {
        it('invokes GET /devices/:app/configs', async function () {
            const name = 'hello-world'
            const request = createApiRequest()
                .get(`/devices/${this.deviceName}/configs`)
                .reply(200, [])

            await listCommand({}, { deviceName: this.deviceName });
            expect(request.isDone()).to.be.true;
        })
    })
})
