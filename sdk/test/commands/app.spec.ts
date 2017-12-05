import { } from 'mocha';
import { expect } from 'chai';
import * as child_process from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import * as nock from 'nock';
import * as sinon from 'sinon';
import * as mockfs from 'mock-fs';
import { CONFIG_FILES, login, createApp, createApiRequest } from '../helpers';

const { create: createCommand, list: listCommand, delete_: deleteCommand } = require('../../dist/commands/app')

describe('app * commands', function() {
    beforeEach(function () {
        mockfs(Object.assign({}, CONFIG_FILES));
        login();
    })

    afterEach(function () {
        mockfs.restore()
        nock.cleanAll()
    })

    describe('app create command', function () {
        it('invokes POST /apps', async function () {
            const name = 'hello-world'
            const request = createApiRequest().post('/apps', { app: { name } }).reply(200, {})

            await createCommand({ name }, {});
            expect(request.isDone()).to.be.true;
        })
    })

    describe('app create command', function () {
        it('invokes POST /apps/:name', async function () {
            const name = 'hello-world'
            const request = createApiRequest().delete(`/apps/${name}`).reply(200, {})

            await deleteCommand({ name }, {});
            expect(request.isDone()).to.be.true;
        })
    })

    describe('app list command', function () {
        it('invokes GET /apps', async function () {
            const name = 'hello-world'
            const request = createApiRequest().get('/apps').reply(200, [])

            await listCommand({}, {});
            expect(request.isDone()).to.be.true;
        })
    })
})
