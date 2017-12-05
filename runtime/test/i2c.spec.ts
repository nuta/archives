import { } from 'mocha';
import * as mockfs from 'mock-fs';
import { expect } from 'chai';
import * as sinon from 'sinon';
import * as fs from 'fs';

process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3';
const { builtins } = require('..');

describe('I2C API', function() {
    beforeEach(function() {
        this.address = 0x3e
        this.atime = new Date(0)
        this.deviceFile = '/dev/i2c-1'

        mockfs({
            '/dev/i2c-1': mockfs.file({ content: '', atime: this.atime })
        })

        this.ioctl = sinon.spy(require('../dist/native'), 'ioctl')
        this.instance = new builtins.I2C({ address: this.address })
    })

    afterEach(function() {
        mockfs.restore()
        this.ioctl.restore()
    })

    describe('read', function() {
        it('access the device file', function() {
            this.instance.read(8);
            expect(fs.statSync(this.deviceFile).atime).to.not.equal(this.atime)
            expect(this.ioctl.called).to.be.true
        })
    })

    describe('write', function() {
        it('access the device file', function() {
            this.instance.write([0x11, 0x22]);
            expect(fs.statSync(this.deviceFile).atime).to.not.equal(this.atime)
            expect(this.ioctl.called).to.be.true
        })
    })
})
