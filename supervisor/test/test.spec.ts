import { } from 'mocha';
import { expect } from 'chai';
import * as nock from 'nock';
import * as mockfs from 'mock-fs';
import * as fs from 'fs';
import { useFakeTimers } from 'sinon';
import {
    createAppImage, computeImageHMAC,
    createHeartbeatResponse, createAppImageResponse, createOSImageResponse,
    prepareAppDir, createSupervisor,
} from './helpers';

describe('Supervisor', function () {
    beforeEach(async function () {
        this.appVersion = '9'

        const { hmac: appImageHMAC, image: appZip } = await createAppImage({
            '/start.js': 'process.send({ type: "log", body: "I am your app, Luke." })'
        })
        this.appImageHMAC = appImageHMAC

        // The response to the first heartbeat.
        this.heartbeatRequest1 = createHeartbeatResponse({
            appVersion: this.appVersion,
            appImageHMAC: this.appImageHMAC,
            stores: {}
        })

        createAppImageResponse(this.appVersion, appZip)
        prepareAppDir()
        this.clock = useFakeTimers(Date.now())
        this.instance = createSupervisor()
    })

    afterEach(function () {
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

    it('downloads and updates the OS', function (done) {
        mockfs({
            '/boot/kernel7.img': ''
        })

        this.instance.start().then(async () => {
            const osVersion = 'v20.1.2'
            const osImage = Buffer.from('This is a new os image!')
            const osImageHMAC = computeImageHMAC(osImage)
            this.heartbeatRequest2 = createHeartbeatResponse({ osVersion, osImageHMAC })
            this.osImageRequest = createOSImageResponse(
                this.instance.osType, this.instance.deviceType, osVersion, osImage)

            // FIXME: This calls an internal method because setInterval in
            // start() won't work well. Refer https://github.com/sinonjs/lolex/pull/105
            await this.instance.doIntervalJob()
            this.clock.next()

            expect(this.heartbeatRequest2.isDone()).to.be.true
            expect(this.osImageRequest.isDone()).to.be.true
            expect(osImage.equals(fs.readFileSync('/boot/kernel7.img'))).to.be.true
            mockfs.restore();
            done()
        })
    })

    it('handles store message properly', function (done) {
        this.instance.start().then(() => {
            // The response to the second heartbeat.
            this.heartbeatRequest2 = createHeartbeatResponse({
                appVersion: this.appVersion,
                appImageHMAC: this.appImageHMAC,
                stores: {
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
