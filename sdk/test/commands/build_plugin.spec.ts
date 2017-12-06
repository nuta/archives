import {} from 'mocha';
import { expect } from 'chai';
import * as child_process from 'child_process';
import * as fs from 'fs';
import * as os from 'os';
import * as path from 'path';
import * as nock from 'nock';
import * as sinon from 'sinon';
import * as mockfs from 'mock-fs';
import {
    CONFIG_FILES, login, createApp, createApiRequest, createZip
} from '../helpers';

const buildPluginCommand = require('../../dist/commands/build_plugin').main

describe('build-plugin command', function() {
    beforeEach(function() {
        mockfs(CONFIG_FILES);
        this.spawnSync = sinon.stub(child_process, 'spawnSync').returns({ status: 0 })
    })

    afterEach(function () {
        mockfs.restore();
        this.spawnSync.restore();
    })

    it('works', async function() {
        // FIXME: mock-fs does not support fs.copyFileSync used in buildPlugin()
        /*
        await buildPluginCommand({
            path: 'foo',
            dest: path.join(os.tmpdir(), 'makestack-build-plugin-test')
        })

        // docker build
        expect(this.spawnSync.firstCall.calledWith('docker')).to.be.true

        // docker run
        expect(this.spawnSync.secondCall.calledWith('docker')).to.be.true
        */
    })
})
