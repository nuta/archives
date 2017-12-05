import { } from 'mocha';
import * as mockfs from 'mock-fs';
import { expect } from 'chai';
import * as sinon from 'sinon';
import * as fs from 'fs';

process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3';
const { builtins } = require('..');


describe('SPI API', function() {
    beforeEach(function() {
        this.slave = 1
        this.atime = new Date(0)
        this.deviceFile = '/dev/spidev0.1'

        mockfs({
            '/dev/spidev0.1': mockfs.file({ atime: this.atime })
        })

        this.spiConfigure = sinon.spy(require('../dist/native'), 'spiConfigure')
        this.spiTransfer = sinon.spy(require('../dist/native'), 'spiTransfer')
        this.instance = new builtins.SPI({ slave: this.slave })
    })

    afterEach(function() {
        mockfs.restore()
        this.spiConfigure.restore()
        this.spiTransfer.restore()
    })

    describe('transfer', function() {
        it('access the device file', function () {
            this.instance.configure('MODE3', 8, 100000);
            this.instance.transfer([0x11, 0x22, 0x33]);
            expect(fs.statSync(this.deviceFile).atime).to.not.equal(this.atime)
            expect(this.spiConfigure.called).to.be.true
            expect(this.spiTransfer.called).to.be.true
        })
    })
})
