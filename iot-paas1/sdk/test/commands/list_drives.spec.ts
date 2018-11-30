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

const listDrivesCommand = require('../../dist/commands/list_drives').main

describe('list-drives command', function() {
    beforeEach(function() {
        mockfs({});
        const drives = [
            {
                displayName: 'Hard Disk',
                description: '',
                size: 3 * 1024 * 1024 * 1024 * 1024,
                device: '/dev/sda1',
                system: true
            },
            {
                displayName: 'USB Memory',
                description: '',
                size: 16 * 1024 * 1024 * 1024,
                device: '/dev/sdy1',
                system: false
            },
            {
                displayName: 'SD Card Reader',
                description: '',
                size: 8 * 1024 * 1024 * 1024,
                device: '/dev/sdz1',
                system: false
            }
        ]
        this.drivelist = sinon.stub(require('drivelist'), 'list')
            .callsFake(callback => callback(null, drives))
    })

    afterEach(function () {
        mockfs.restore();
        this.drivelist.restore()
    })

    it('works', async function() {
        await listDrivesCommand()
    })
})
