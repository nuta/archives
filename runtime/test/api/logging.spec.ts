import { } from 'mocha';
import { expect } from 'chai';
import * as sinon from 'sinon';

process.env.MAKESTACK_ENV = 'test';
process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3';
const { println, eprintln } = require('../..');

describe('Logging API', function() {
    beforeEach(function () {
        process.send = () => {}
        this.processSend = sinon.spy(process, 'send')
    })

    afterEach(function() {
        this.processSend.restore()
    })

    describe('println', function() {
        it('calls process.send', function () {
            const message = 'Hello, World!'
            println(message);
            expect(this.processSend.calledWith({ type: "log", body: message })).to.be.true;
        })
    })

    describe('eprintln', function() {
        it('calls process.send', function () {
            const message = 'Something went wrong!'
            eprintln(message);
            expect(this.processSend.calledWith({ type: "log", body: `!${message}` })).to.be.true;
        })
    })
})
