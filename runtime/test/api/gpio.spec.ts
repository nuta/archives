import { } from 'mocha';
import * as mockfs from 'mock-fs';
import * as sinon from 'sinon';
import { expect } from 'chai';
import * as fs from 'fs';

process.env.MAKESTACK_ENV = 'test';
process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3';
import { GPIO } from '../..';

function readFile(path: string) {
    return fs.readFileSync(path, { encoding: 'utf-8' })
}

describe('GPIO API', function() {
    beforeEach(function() {
        this.pin = 1
        mockfs({
            '/sys/class/gpio/export': '',
            '/sys/class/gpio/gpio1/direction': ''
        })

        this.fsWatch = sinon.stub(fs, 'watch')
        this.instance = new GPIO({
            pin: this.pin,
            mode: 'out'
        })
    })

    afterEach(function() {
        mockfs.restore()
        this.fsWatch.restore()
    })

    describe('setMode', function() {
        it('changes the direction', function() {
            this.instance.setMode('in');
            expect(readFile('/sys/class/gpio/export')).to.equal('1');
            expect(readFile('/sys/class/gpio/gpio1/direction')).to.equal('in');
        })
    })

    describe('write', function() {
        it('changes the value', function() {
            this.instance.write(true);
            expect(readFile('/sys/class/gpio/gpio1/value')).to.equal('1');
        })
    })

    describe('read', function() {
        it('reads the value', function() {
            this.instance.write(false);
            expect(this.instance.read()).to.equal(false);
        })
    })

    describe('onInterrupt', function() {
        it('calls fs.watch', function () {
            const callback = sinon.stub()
            this.instance.onInterrupt(callback);
            expect(this.fsWatch.called).to.be.true;
        })
    })

    describe('onChange', function() {
        it('calls fs.watch', function () {
            const callback = sinon.stub()
            this.instance.onChange(callback);
            expect(this.fsWatch.called).to.be.true;
        })
    })
})
