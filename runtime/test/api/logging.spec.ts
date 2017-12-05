import { } from 'mocha';
import { expect } from 'chai';
import * as sinon from 'sinon';

process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3';
const { builtins } = require('../..');

describe('Logging API', function() {
    beforeEach(function () {
        process.send = () => {}
        this.processSend = sinon.spy(process, 'send')
    })

    afterEach(function() {
        this.processSend.restore()
    })

    describe('publish', function() {
        it('calls process.send', function () {
            const message = 'Hello, World!'
            builtins.println(message);
            expect(this.processSend.calledWith({ type: "log", body: message })).to.be.true;
        })
    })

    describe('error', function() {
        it('calls process.send', function () {
            const message = 'Something went wrong!'
            builtins.error(message);
            expect(this.processSend.calledWith({ type: "log", body: `!${message}` })).to.be.true;
        })
    })
})
