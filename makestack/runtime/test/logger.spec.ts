import { } from 'mocha';
import * as mockfs from 'mock-fs';
import { expect } from 'chai';
import * as sinon from 'sinon';
import * as fs from 'fs';

process.env.MAKESTACK_ENV = 'test';
process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3';

describe('Logger', function() {
    beforeEach(function () {
        this.logbuf = ''
        this.logger = require('../dist/logger').logger
        this.log = sinon.stub(console, 'log').callsFake(log => this.logbuf += log + '\n')
    })

    afterEach(function() {
        this.log.restore()
    })

    describe('debug', function() {
        it('calls console.log', function() {
            this.logger.debug('hello', ['a', 'b', 'c']);
            expect(this.logbuf).to.include('hello');
            expect(this.logbuf).to.include("[ 'a', 'b', 'c' ]");
        })
    })

    describe('info', function() {
        it('calls console.log', function() {
            this.logger.info('hello', ['a', 'b', 'c']);
            expect(this.logbuf).to.include('hello');
            expect(this.logbuf).to.include("[ 'a', 'b', 'c' ]");
        })
    })

    describe('warn', function() {
        it('calls console.log', function() {
            this.logger.warn('hello', ['a', 'b', 'c']);
            expect(this.logbuf).to.include('hello');
            expect(this.logbuf).to.include("[ 'a', 'b', 'c' ]");
        })
    })

    describe('error', function() {
        it('calls console.log', function() {
            this.logger.error('hello', ['a', 'b', 'c']);
            expect(this.logbuf).to.include('hello');
            expect(this.logbuf).to.include("[ 'a', 'b', 'c' ]");
        })
    })
})
