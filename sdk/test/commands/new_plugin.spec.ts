import {} from 'mocha';
import { expect } from 'chai';
import * as child_process from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import * as nock from 'nock';
import * as sinon from 'sinon';
import * as mockfs from 'mock-fs';
import { CONFIG_FILES } from '../helpers';

const CONFIG_DIR = '/user/.makestack'
process.env.CONFIG_DIR = CONFIG_DIR
const newPluginCommand = require('../../dist/commands/new_plugin').main

describe('new-plugin command', function() {
    beforeEach(function() {
        this.spawnSync = sinon.stub(child_process, 'spawnSync').returns({ status: 0 })
        this.pluginDir = '/user/plugin';
        mockfs(CONFIG_FILES)
    })

    afterEach(function () {
        this.spawnSync.restore()
        mockfs.restore()
        nock.cleanAll()
    })

    it('creates a new plugin directory', async function() {
        await newPluginCommand({ dir: this.pluginDir }, { lang: 'javascript' });
        expect(fs.existsSync(path.join(this.pluginDir, 'README.md'))).to.be.true;
        expect(fs.existsSync(path.join(this.pluginDir, 'lib/index.js'))).to.be.true;
        expect(fs.existsSync(path.join(this.pluginDir, 'plugin.yaml'))).to.be.true;
        expect(fs.existsSync(path.join(this.pluginDir, '.makestackignore'))).to.be.true;
        expect(fs.existsSync(path.join(this.pluginDir, 'package.json'))).to.be.true;
        expect(fs.existsSync(path.join(this.pluginDir, 'jsconfig.json'))).to.be.true;
    })
})
