import { } from 'mocha';
import { expect } from 'chai';
import * as sinon from 'sinon';

process.env.MAKESTACK_ENV = 'test';
process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3';
const { Timer } = require('../..');

describe('Timer API', function() {
    beforeEach(function () {
        this.timers = sinon.useFakeTimers()
    })

    afterEach(function() {
        this.timers.restore()
    })

    describe('interval', function() {
        it('calls setInterval', function () {
            const callback = sinon.stub();
            Timer.interval(3, callback);
            this.timers.tick(3 * 1000 + 1);
            expect(callback.calledOnce).to.be.true;
        })
    })

    describe('sleep', function() {
        it('resolves a promise after the duration', function (done) {
            Timer.sleep(3).then(done)
            this.timers.tick(3 * 1000 + 1);
        })
    })

    describe('delay', function() {
        it('calls a callback after the duration', function (done) {
            Timer.delay(3, done)
            this.timers.tick(3 * 1000 + 1);
        })
    })
})

describe('Timer API (without fake timers)', function () {
    describe('busywait', function() {
        it('returns after the duration', function () {
            Timer.busywait(10)
        })
    })
})
