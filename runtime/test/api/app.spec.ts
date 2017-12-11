import { } from 'mocha';
import { expect } from 'chai';
import * as sinon from 'sinon';

process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3';
const { App } = require('../..');

describe('App API', function() {
    beforeEach(function () {
        process.send = () => {}
        this.processSend = sinon.spy(process, 'send')
        this.processOn = sinon.spy(process, 'on')
    })

    afterEach(function() {
        this.processSend.restore()
        this.processOn.restore()
    })

    describe('disableUpdate', function() {
        it('calls process.send', function () {
            App.disableUpdate();
            expect(this.processSend.calledWith({ type: "setUpdateLock", body: "lock" })).to.be.true;
        })
    })

    describe('enableUpdate', function() {
        it('calls process.send', function () {
            App.enableUpdate();
            expect(this.processSend.calledWith({ type: "setUpdateLock", body: "unlock" })).to.be.true;
        })
    })

    describe('onExit', function() {
        it('calls process.on', function () {
            App.onExit(sinon.mock());
            expect(this.processOn.calledWith('SIGTERM')).to.be.true;
        })
    })
})
