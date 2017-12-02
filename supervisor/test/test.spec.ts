import { } from 'mocha';
import { expect } from 'chai';
import { useFakeTimers } from 'sinon';
import {
  createAppImage,
  createHeartbeatResponse, createImageResponse,
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

    createImageResponse(this.appVersion, appZip)
    prepareAppDir()
    this.clock = useFakeTimers(Date.now())
    this.instance = createSupervisor()
  })

  afterEach(function () {
    this.clock.restore()
  })

  it('generates a heartbeat request', function (done) {
    this.instance.start().then(() => {
      expect(this.heartbeatRequest1.isDone()).to.be.true
      done()
    })
  })

  it('downloads and run an app', function (done) {
    this.instance.start().then(() => {
      this.instance.waitForApp().then(log => {
        expect(log).to.include('I am your app, Luke.')
        done()
      })
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

      this.instance.waitForApp().then(log => {
        expect(this.heartbeatRequest2.isDone()).to.be.true
        expect(log).to.include('I am your app, Luke.')
        done()
      })
    })
  })
})
