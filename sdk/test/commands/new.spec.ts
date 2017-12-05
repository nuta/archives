import { expect } from 'chai';
import { spawnSync } from 'child_process';
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
        const makestackDTSPath = path.resolve(__dirname, '../../runtime/makestack.d.ts')
        mockfs({
            '/user': {},
            [makestackDTSPath]: fs.readFileSync(makestackDTSPath)
        })
    })

    afterEach(function() {
        mockfs.restore()
        nock.cleanAll()
    })

    it('creates a new app directoy', async function() {
        await newCommand({ dir: this.appDir }, { plugins: '' });
        expect(fs.existsSync(path.join(this.appDir, 'README.md'))).to.be.true;
        expect(fs.existsSync(path.join(this.appDir, 'app.js'))).to.be.true;
        expect(fs.existsSync(path.join(this.appDir, 'app.yaml'))).to.be.true;
        expect(fs.existsSync(path.join(this.appDir, 'jsconfig.json'))).to.be.true;
    })
})
