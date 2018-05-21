import { } from 'mocha';
import { expect } from 'chai';
import * as sinon from 'sinon';

process.env.MAKESTACK_ENV = 'test';
process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3';
const { print, eprint } = require('../..');

describe('Logging API', function() {
    beforeEach(function () {
        process.send = () => {}
        this.processSend = sinon.spy(process, 'send')
    })

    afterEach(function() {
        this.processSend.restore()
    })

    describe('print', function() {
        it('calls process.send', function () {
            const message = 'Hello, World!'
            print(message);
            expect(this.processSend.calledWith({ type: "log", body: message })).to.be.true;
        })
    })

    describe('eprint', function() {
        it('calls process.send', function () {
            const message = 'Something went wrong!'
            eprint(message);
            expect(this.processSend.calledWith({ type: "log", body: `!${message}` })).to.be.true;
        })
    })
})
