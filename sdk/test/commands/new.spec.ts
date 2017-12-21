import {} from 'mocha';
import { expect } from 'chai';
import * as sinon from 'sinon';
import * as childProcess from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import * as nock from 'nock';
import * as mockfs from 'mock-fs';

const CONFIG_DIR = '/user/.makestack'
process.env.CONFIG_DIR = CONFIG_DIR
const newCommand = require('../../dist/commands/new').main

describe('new command', function() {
    beforeEach(function() {
        this.appDir = '/user/app'
        this.spawnSync = sinon.stub(childProcess, 'spawnSync').returns({ status: 0 })
        mockfs({
            '/user': {}
        })
    })

    afterEach(function() {
        mockfs.restore()
        nock.cleanAll()
        this.spawnSync.restore()
    })

    it('creates a new app directoy', async function() {
        await newCommand({ dir: this.appDir }, { plugins: '' });
        expect(fs.existsSync(path.join(this.appDir, 'README.md'))).to.be.true;
        expect(fs.existsSync(path.join(this.appDir, 'app.js'))).to.be.true;
        expect(fs.existsSync(path.join(this.appDir, 'package.json'))).to.be.true;
        expect(fs.existsSync(path.join(this.appDir, 'jsconfig.json'))).to.be.true;
    })
})
