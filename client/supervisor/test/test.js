const assert = require('power-assert')
const nock = require('nock')
const mockfs = require('mock-fs')
const sinon = require('sinon')
const path = require('path')

const deviceId = 'abc'
const deviceSecret = '123'
process.env.MAKESTACK_DEVICE_TYPE = 'raspberrypi3'
process.env.RUNTIME_MODULE = path.resolve(__dirname, '../../nodejs-runtime')
const Supervisor = require('..')

const serverURL = 'http://test-serverURL'

describe('Supervisor', function () {
  beforeEach(function () {
    const adapter = 'http'
    const osVersion = 'c'
    this.heartbeatInterval = 5

    this.heartbeatRequest = nock(serverURL)
      .post('/api/v1/smms', () => true)
      .reply(200, Buffer.from([]))

    this.instance = new Supervisor({
      appDir: path.resolve('/app'),
      adapter: {
        name: adapter,
        url: serverURL
      },
      osType: 'mock',
      deviceType: 'raspberrypi3',
      deviceId: deviceId,
      deviceSecret: deviceSecret,
      debugMode: true,
      osVersion,
      heartbeatInterval: this.heartbeatInterval
    })

    this.clock = sinon.useFakeTimers()

    mockfs({
      'app': {}
    })
  })

  afterEach(function () {
    mockfs.restore()
    this.clock.restore()
  })

  it('generates ', function () {
    this.instance.start()
    this.clock.tick(this.heartbeatInterval * 1000 + 1)
    this.heartbeatRequest.isDone()
  })
})
