import { } from 'mocha';
import * as mockfs from 'mock-fs';
import { expect } from 'chai';
import * as fs from 'fs';

process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3';
const { builtins } = require('..');

function readFile(path) {
    return fs.readFileSync(path, { encoding: 'utf-8' })
}

describe('GPIO API', function() {
    beforeEach(function() {
        this.pin = 1
        mockfs({
            '/sys/class/gpio/export': '',
            '/sys/class/gpio/gpio1/direction': ''
        })

        this.instance = new builtins.GPIO({
            pin: this.pin,
            mode: 'out'
        })
    })

    afterEach(function() {
        mockfs.restore()
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
})
