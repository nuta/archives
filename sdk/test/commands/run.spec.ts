import {} from 'mocha';
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

const runCommand = require('../../dist/commands/run').main

describe('run command', function() {
    beforeEach(function() {
        this.supervisor = sinon.stub(require('../../supervisor'), 'Supervisor')
            .returns({
                start() { }
            })
        mockfs({});
    })

    afterEach(function () {
        mockfs.restore();
        this.supervisor.restore();
    })

    it('uses Supervisor', async function () {
        const server = 'http://makestack-server'
        await runCommand({}, { adapter: 'http', server })
        expect(this.supervisor.calledOnce).to.be.true
    })
})
