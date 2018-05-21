import { } from 'mocha';
import { expect } from 'chai';
import * as nock from 'nock';
import * as mockfs from 'mock-fs';
import * as fs from 'fs';
import { useFakeTimers } from 'sinon';
import {
    createAppImage,
    createHeartbeatResponse, createAppImageResponse,
    prepareAppDir, createSupervisor,
} from './helpers';

describe('Supervisor', function () {
    beforeEach(async function () {
        this.appVersion = 9

        const { image: appZip } = await createAppImage({
            '/start.js': 'process.send({ type: "log", body: "I am your app, Luke." })'
        })

        // The response to the first heartbeat.
        this.heartbeatRequest1 = createHeartbeatResponse({
            update: { version: this.appVersion }
        })

        createAppImageResponse(this.appVersion, appZip)
        prepareAppDir()
        this.clock = useFakeTimers(Date.now())
        this.instance = createSupervisor()
    })

    afterEach(function () {
        this.instance.destroy();
        nock.cleanAll();
        this.clock.restore();
    })

    it('generates a heartbeat request', function (done) {
        this.instance.start().then(() => {
            expect(this.heartbeatRequest1.isDone()).to.be.true
            done()
        })
    })

    it('downloads and run an app', function (done) {
        this.instance.start().then(() => {
            this.instance.waitForApp().then((log: string) => {
                expect(log).to.include('I am your app, Luke.')
                done()
            })
        })
    })

    it('handles store message properly', function (done) {
        this.instance.start().then(() => {
            // The response to the second heartbeat.
            this.heartbeatRequest2 = createHeartbeatResponse({
                update: { version: this.appVersion },
                configs: {
                    'display-message': 'ALIVE!'
                }
            })

            // Invoke a heartbeating.
            this.clock.tick(this.instance.heartbeatInterval * 1000 + 1)

            this.instance.waitForApp().then((log: string) => {
                expect(this.heartbeatRequest2.isDone()).to.be.true
                expect(log).to.include('I am your app, Luke.')
                done()
            })
        })
    })
})
