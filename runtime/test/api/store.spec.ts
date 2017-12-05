import { } from 'mocha';
import { expect } from 'chai';
import * as sinon from 'sinon';

process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3';
const { builtins } = require('../..');

describe('Store API', function() {
    beforeEach(function () {
        process.send = () => {}
        this.processSend = sinon.spy(process, 'send')
    })

    afterEach(function() {
        this.processSend.restore()
    })

    describe('onCommand', function() {
        it('calls the callback', async function () {
            const cmd = 'move_forward'
            const arg = '5'
            const callback = sinon.stub()
            builtins.Store.onCommand(cmd, callback)
            await builtins.Store.update({ [`>1 ${cmd}`]: arg })

            expect(callback.calledWith(arg)).to.be.true
        })
    })

    describe('onChange', function() {
        it('calls the callback', async function () {
            const key = 'display_message'
            const data = "Today's high will be 13 degrees."
            const callback = sinon.stub()
            await builtins.Store.update({ [key]: '' })
            builtins.Store.onChange(key, callback)
            await builtins.Store.update({ [key]: data })

            expect(callback.calledWith(data)).to.be.true
        })
    })
})
