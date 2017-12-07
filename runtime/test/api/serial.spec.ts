import { } from 'mocha';
import * as mockfs from 'mock-fs';
import { expect } from 'chai';
import * as sinon from 'sinon';
import * as fs from 'fs';

process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3';
const { builtins } = require('../..');

describe('Serial API', function() {
    beforeEach(function() {
        this.address = 0x3e
        this.atime = new Date(0)
        this.deviceFile = '/dev/ttyAMA0'
        this.writtenData = "$ABC,123.123,456.789\n$ABD,916.552,566.231"

        mockfs({
            [this.deviceFile]: mockfs.file({ content: this.writtenData, atime: this.atime }),
            '/dev/ttyUSB0': ''
        })

        this.serialConfigure = sinon.stub(require('../../dist/native').functions, 'serialConfigure')
        this.clock = sinon.useFakeTimers()
        this.instance = new builtins.Serial({ path: this.deviceFile, baudrate: 115200 })
    })

    afterEach(function() {
        mockfs.restore();
        this.clock.restore();
        this.serialConfigure.restore();
    })

    describe('list', function() {
        it('returns a list of available device files', function() {
            expect(builtins.Serial.list()).to.deep.equal([this.deviceFile, '/dev/ttyUSB0']);
        })
    })

    describe('read', function() {
        it('returns read data', function() {
            const data = this.instance.read();
            expect(data.equals(Buffer.from(this.writtenData))).to.true
        })
    })

    describe('write', function() {
        it('writes data to the device file', function () {
            const data = Buffer.from("hello");
            this.instance.write(data);
            expect(fs.readFileSync(this.deviceFile).includes(data)).to.be.true
        })
    })

    describe('onData', function() {
        it('calls the callback', function () {
            const callback = sinon.stub();
            this.instance.onData(callback);

            // Assumes that .read() returns this.writtenData and onData
            // calls the callback with the returned string as a buffer.
            this.clock.next();

            expect(callback.calledWith(Buffer.from(this.writtenData))).to.be.true;
        })
    })

    describe('onNewLine', function() {
        it('calls the callback', function () {
            const callback = sinon.stub();
            this.instance.onNewLine(callback);

            // Assumes that .read() returns this.writtenData and onNewLine
            // calls the callback with the returned string as a buffer.
            this.clock.next();

            const lines = this.writtenData.split('\n')
            expect(callback.firstCall.calledWith(lines[0])).to.be.true;
            expect(callback.secondCall).to.be.null; // writtenData is not terminated by '\n'
        })
    })
})
