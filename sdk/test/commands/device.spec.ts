import { expect } from 'chai';
import * as child_process from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import * as nock from 'nock';
import * as sinon from 'sinon';
import * as mockfs from 'mock-fs';
import { CONFIG_FILES, login, createApp, createApiRequest } from '../helpers';

const { create: createCommand, list: listCommand, delete_: deleteCommand } = require('../../dist/commands/device')

describe('device * commands', function() {
    beforeEach(function () {
        mockfs(Object.assign({}, CONFIG_FILES));
        login();
    })

    afterEach(function () {
        mockfs.restore()
        nock.cleanAll()
    })

    describe('device create command', function () {
        it('invokes POST /devices/:name', async function () {
            const name = 'hello-world'
            const request = createApiRequest().delete(`/devices/${name}`).reply(200, {})

            await deleteCommand({ name }, {});
            expect(request.isDone()).to.be.true;
        })
    })

    describe('device list command', function () {
        it('invokes GET /devices', async function () {
            const name = 'hello-world'
            const request = createApiRequest().get('/devices').reply(200, [])

            await listCommand({}, {});
            expect(request.isDone()).to.be.true;
        })
    })
})
