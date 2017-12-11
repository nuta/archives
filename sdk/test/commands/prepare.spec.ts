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

const prepareCommand = require('../../dist/commands/prepare').main

describe('prepare command', function() {
    beforeEach(function () {
        mockfs(Object.assign({}, CONFIG_FILES));
        this.appDir = createApp();
        login();
    })

    afterEach(function () {
        mockfs.restore()
        nock.cleanAll()
    })

    it('works', async function () {
        await prepareCommand({}, { appDir: this.appDir });
    })
})
