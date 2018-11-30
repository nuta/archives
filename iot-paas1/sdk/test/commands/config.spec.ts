import { } from 'mocha';
import { expect } from 'chai';
import * as child_process from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import * as nock from 'nock';
import * as sinon from 'sinon';
import * as mockfs from 'mock-fs';
import { CONFIG_FILES, login, createApp, createApiRequest } from '../helpers';

const { set: setCommand, list: listCommand, delete_: deleteCommand } = require('../../dist/commands/config')

describe('config * commands', function() {
    beforeEach(function () {
        mockfs(Object.assign({}, CONFIG_FILES));
        this.appName = 'display';
        this.appDir = createApp(this.appName);
        login();
    })

    afterEach(function () {
        mockfs.restore()
        nock.cleanAll()
    })

    describe('config set command', function () {
        it('invokes PUT /apps/:app/configs/:name', async function () {
            const name = 'message'
            const dataType = 'string'
            const value = 'hello!'
            const request = createApiRequest()
                .put(`/apps/${this.appName}/configs/${name}`,
                    { config: { value, data_type: dataType } })
                .reply(200, {})

            await setCommand({ name, value }, { appDir: this.appDir, type: dataType });
            expect(request.isDone()).to.be.true;
        })
    })

    describe('config create command', function () {
        it('invokes POST /apps/:app/configs/:name', async function () {
            const name = 'message'
            const request = createApiRequest()
                .delete(`/apps/${this.appName}/configs/${name}`)
                .reply(200, {})

            await deleteCommand({ name }, { appDir: this.appDir });
            expect(request.isDone()).to.be.true;
        })
    })

    describe('config list command', function () {
        it('invokes GET /apps/:app/configs', async function () {
            const name = 'hello-world'
            const request = createApiRequest()
                .get(`/apps/${this.appName}/configs`)
                .reply(200, [])

            await listCommand({}, { appDir: this.appDir });
            expect(request.isDone()).to.be.true;
        })
    })
})
