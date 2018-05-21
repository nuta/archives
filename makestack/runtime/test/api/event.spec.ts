import { } from 'mocha';
import { expect } from 'chai';
import * as sinon from 'sinon';

process.env.MAKESTACK_ENV = 'test';
process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3';
const { publish } = require('../..');

describe('Event API', function() {
    beforeEach(function () {
        process.send = () => {}
        this.processSend = sinon.spy(process, 'send')
    })

    afterEach(function() {
        this.processSend.restore()
    })

    describe('publish', function() {
        it('calls process.send', function () {
            const event = 'sensing'
            const data = 'button=pressed temp=22.12 humid=80.23'
            publish(event, data);
            expect(this.processSend.calledWith({ type: "log", body: `@${event} ${data}` })).to.be.true;
        })
    })
})
