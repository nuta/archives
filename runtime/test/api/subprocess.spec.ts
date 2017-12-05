import { } from 'mocha';
import * as mockfs from 'mock-fs';
import { expect } from 'chai';
import * as sinon from 'sinon';
import * as child_process from 'child_process';

process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3';
const { builtins } = require('../..');

describe('Subprocess API', function() {
    beforeEach(function () {
        this.spawnSync = sinon.spy(child_process, 'spawnSync');
    })

    afterEach(function() {
        this.spawnSync.restore()
    })

    describe('debug', function() {
        it('calls child_process.SpawnSync', function () {
            const exe = './gopher';
            const args = ['go', 'Gopher!']
            builtins.SubProcess.run([exe, ...args]);
            expect(this.spawnSync.calledWith(exe, args)).to.be.true;
        })
    })
})
