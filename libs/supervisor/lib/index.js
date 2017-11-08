const { fork, spawnSync } = require('child_process')
const assert = require('assert')
const fs = require('fs')
const os = require('os')
const crypto = require('crypto')
const path = require('path')
const HTTPAdapter = require('./adapters/http')
const SakuraioAdapter = require('./adapters/sakuraio')
const logger = require('./logger')

const SMMS_VERSION = 1
const SMMS_HMAC_MSG = 0x06
const SMMS_TIMESTAMP_MSG = 0x07
const SMMS_DEVICE_ID_MSG = 0x0a
const SMMS_DEVICE_INFO_MSG = 0x0b
const SMMS_LOG_MSG = 0x0c
const SMMS_OS_VERSION_MSG = 0x10
const SMMS_APP_VERSION_MSG = 0x11
const SMMS_OS_IMAGE_HMAC_MSG = 0x12
const SMMS_APP_IMAGE_HMAC_MSG = 0x13
const SMMS_STORE_MSG = 0x20

class Supervisor {
  constructor({ adapter, appDir, deviceType, osVersion, deviceId, deviceSecret, debugMode, appUID, appGID }) {
    this.app = null
    this.appDir = appDir
    this.osVersion = osVersion
    this.debugMode = debugMode
    this.appUID = parseInt(appUID) || undefined
    this.appGID = parseInt(appGID) || undefined
    this.deviceId = deviceId
    this.deviceSecret = deviceSecret
    this.deviceType = deviceType
    this.device = new (require(`./devices/${deviceType}`))()
    this.appVersion = 'X'
    this.log = ''
    this.stores = {}
    this.adapterName = adapter.name

    process.env.DEVICE_TYPE = deviceType
    this.appAPI = require('app-runtime')

    switch (this.adapterName) {
      case 'http':
        this.adapter = new HTTPAdapter(this.deviceId, adapter.url)
        this.verifyHMAC = true
        break
      case 'sakuraio':
        this.adapter = new SakuraioAdapter(this.appAPI.I2C)
        this.verifyHMAC = false
        break
      default:
        throw new Error(`unknown adapter \`${this.adapterName}'`)
    }
  }

  popLog() {
    const log = this.log
    this.log = ''
    return log
  }

  updateOS(image) {
    logger.info('saving os image...')
    const tmpFilePath = path.join(os.tmpdir(), 'kernel.img')
    fs.writeFileSync(tmpFilePath, image)

    logger.warn('sending SIGTERM to the app...')
    if (this.app) {
      this.app.kill()
    }

    // Wait the app to exit.
    logger.warn('OS will be updated soon!')
    setTimeout(() => {
      logger.info('updating os image...')
      this.device.updateOS(tmpFilePath)
      logger.info('updateOS returned!')
      this.adapter.send(this.serialize({
        state: 'ready',
        deviceId: this.deviceId,
        debugMode: this.debugMode,
        osVersion: this.osVersion,
        appVersion: 0,
        log: 'os updated'
      }))
    }, 5000)
  }

  launchApp(appZip) {
    const appZipPath = path.join(os.tmpdir(), 'app.zip')

    fs.writeFileSync(appZipPath, appZip)
    spawnSync('rm', ['-r', this.appDir])
    spawnSync('mkdir', ['-p', this.appDir])
    spawnSync('unzip', ['-q', appZipPath, '-d', this.appDir], {
      stdio: 'inherit'
    })

    this.spawnApp(this.appDir)
  }

  spawnApp(appDir) {
    if (this.app) {
      logger.info('killing the app')
      this.app.kill()
    }

    this.app = fork('./start', [], {
      cwd: appDir,
      stdio: 'inherit',
      uid: this.appUID,
      gid: this.appGID,
      env: {
        DEVICE_TYPE: this.deviceType
      }
    })
    this.sendToApp('initialize', { stores: this.stores })

    this.app.on('message', (data) => {
      logger.info('message', data)
      switch (data.type) {
        case 'log':
          logger.info('device', data.body)
          this.log += data.body
          break
        default:
          logger.info('unknown message', data.type)
      }
    })

    this.app.on('exit', () => {
      this.app = null
      logger.info('app exited')
    })
  }

  sendToApp(type, data) {
    if (!this.app) {
      // The app is being killed.
      return
    }

    this.app.send(Object.assign({ type }, data))
  }

  generateVariableLength(buf) {
    let len = buf.length
    let lenbuf = Buffer.alloc(0)

    while (len > 0) {
      const digit = len % 0x80
      len = Math.floor(len / 0x80)
      buf = Buffer.from([((len > 0) ? 0x80 : 0) | digit])
      lenbuf = Buffer.concat([lenbuf, buf])
    }

    return ((lenbuf.length > 0) ? lenbuf : Buffer.from([0x00]))
  }

  parseVariableLength(buf) {
    let length = 0
    let i = 0
    let base = 1
    while (true) {
      if (i === buf.length) {
        throw new Error('invalid variable length')
      }

      const byte = buf[i]
      length += (byte & 0x7f) * base

      if ((byte & 0x80) === 0) {
        return [length, i + 1]
      }

      i++
      base *= 128
    }
  }

  generateMessage(type, payload) {
    const buf = Buffer.from(payload)
    const lenbuf = this.generateVariableLength(buf)
    const msg = Buffer.alloc(1 + buf.length + 1)
    msg.writeUInt8(type, 0)
    lenbuf.copy(msg, 1) // size
    buf.copy(msg, 1 + lenbuf.length)

    return msg
  }

  serialize(messages, includeHMAC = true) {
    let payload = Buffer.alloc(0)

    if ('deviceId' in messages) {
      const deviceIdMsg = this.generateMessage(SMMS_DEVICE_ID_MSG, messages.deviceId)
      payload = Buffer.concat([payload, deviceIdMsg])
    }

    if ('state' in messages) {
      const states = { booting: 1, ready: 2, running: 3 }

      if (!states[messages.state]) {
        throw new Error(`Invalid device state: \`${messages.state}'`)
      }

      const data = [states[messages.state]]
      const deviceInfoMsg = this.generateMessage(SMMS_DEVICE_INFO_MSG, data)

      payload = Buffer.concat([payload, deviceInfoMsg])
    }

    if ('log' in messages) {
      const logMsg = this.generateMessage(SMMS_LOG_MSG, messages.log)
      payload = Buffer.concat([payload, logMsg])
    }

    if ('osVersion' in messages) {
      const osVersionMsg = this.generateMessage(SMMS_OS_VERSION_MSG, messages.osVersion)
      payload = Buffer.concat([payload, osVersionMsg])
    }

    if ('appVersion' in messages) {
      const appVersionMsg = this.generateMessage(SMMS_APP_VERSION_MSG, messages.appVersion)
      payload = Buffer.concat([payload, appVersionMsg])
    }

    let header = Buffer.alloc(1)
    header.writeUInt8(SMMS_VERSION << 4, 0)

    if (includeHMAC) {
      const timestamp = (new Date()).toISOString()
      const timestampMsg = this.generateMessage(SMMS_TIMESTAMP_MSG, timestamp)
      payload = Buffer.concat([payload, timestampMsg])

      const hmacMsgLength = 1 + 1 + 64 // type, length, sha256sum
      const dummy = Buffer.concat([payload, Buffer.alloc(hmacMsgLength)])
      header = Buffer.concat([header, this.generateVariableLength(dummy)])

      const hmac = this.computeHMAC(Buffer.concat([header, payload]))
      const hmacMsg = this.generateMessage(SMMS_HMAC_MSG, hmac)

      assert.equal(hmacMsgLength, hmacMsg.length)

      payload = Buffer.concat([payload, hmacMsg])
    } else {
      header = Buffer.concat([header, this.generateVariableLength(payload)])
    }

    return Buffer.concat([header, payload])
  }

  deserialize(payload) {
    let version = payload.readUInt8(0)
    if (version >> 4 !== SMMS_VERSION) {
      throw new Error('unsupported smms version')
    }

    const [totalLength, totalLengthLength] = this.parseVariableLength(payload.slice(1))
    const headerLength = 1 + totalLengthLength
    let messages = {}
    let offset = headerLength
    let hmacProtectedEnd = null
    while (offset < headerLength + totalLength) {
      if (hmacProtectedEnd) {
        throw new Error('invalid payload: hmac message must be the last one')
      }

      const type = payload[offset]
      const [length, lengthLength] = this.parseVariableLength(payload.slice(offset + 1))
      const dataOffset = offset + 1 + lengthLength
      const data = payload.slice(dataOffset, dataOffset + length)

      switch (type) {
        case SMMS_STORE_MSG: {
          const keyLengthOffset = dataOffset
          const [keyLength, keyLengthLength] = this.parseVariableLength(payload.slice(keyLengthOffset))
          const keyOffset = keyLengthOffset + keyLengthLength
          const valueOffset = keyOffset + keyLength
          const valueLength = length - (valueOffset - keyLengthOffset)
          const key = payload.slice(keyOffset, keyOffset + keyLength)
          const value = payload.slice(valueOffset, valueOffset + valueLength)

          if (!('stores' in messages)) {
            messages.stores = {}
          }

          messages.stores[key] = value
          break
        }
        case SMMS_OS_VERSION_MSG:
          messages.osVersion = data.toString('utf-8')
          break
        case SMMS_APP_VERSION_MSG:
          messages.appVersion = data.toString('utf-8')
          break
        case SMMS_OS_IMAGE_HMAC_MSG:
          messages.osImageHMAC = data.toString('utf-8')
          break
        case SMMS_APP_IMAGE_HMAC_MSG:
          messages.appImageHMAC = data.toString('utf-8')
          break
        case SMMS_HMAC_MSG:
          messages.hmac = data.toString('utf-8')
          hmacProtectedEnd = offset
          break
        case SMMS_TIMESTAMP_MSG:
          messages.timestamp = data.toString('utf-8')
          break
      }
      offset += 1 + lengthLength + length
    }

    return [messages, hmacProtectedEnd]
  }

  sendHeartbeat(state) {
    logger.info(`heartbeating (state=running, os_ver="${this.osVersion}", ` +
      `app_ver="${this.appVersion}", debug=${this.debugMode})`)

    this.adapter.send(this.serialize({
      state,
      deviceId: this.deviceId,
      debugMode: this.debugMode,
      osVersion: this.osVersion,
      appVersion: this.appVersion,
      log: this.popLog()
    }))
  }

  computeHMAC(data) {
    const hmac = crypto.createHmac('sha256', this.deviceSecret)
    hmac.update(data)
    return hmac.digest('hex')
  }

  verifyMessageHMAC(payload, hmac, timestamp) {
    if (typeof timestamp !== 'string') {
      console.error('timestamp is not set')
      return false
    }

    if ((new Date() - (new Date(timestamp))) > 5 * 60 * 1000 /* msec */) {
      console.error('too old timestamp')
      return false
    }

    if (typeof hmac !== 'string') {
      console.error('hmac is not set')
      return false
    }

    if (hmac !== this.computeHMAC(payload)) {
      console.error('invalid hmac')
      return false
    }

    return true
  }

  verifyImageHMAC(hmac, image) {
    if (!hmac) {
      return false
    }

    const hash = crypto.createHash('sha256')
    hash.update(image)
    const shasum = hash.digest('hex')

    return (hmac === this.computeHMAC(shasum))
  }

  async start() {
    await this.adapter.connect()
    this.sendHeartbeat('ready')

    let downloading = false // XXX: use mutex or something better way
    this.adapter.onReceive(payload => {
      const [messages, hmacProtectedEnd] = this.deserialize(payload)
      const hmacProtectedPayload = payload.slice(0, hmacProtectedEnd)

      if (this.verifyHMAC && !this.verifyMessageHMAC(hmacProtectedPayload,
        messages.hmac, messages.timestamp)) {
        logger.warn('invalid message hmac, discarding...')
        return
      }

      // Update OS.
      if (!downloading && messages.osVersion && messages.osVersion !== this.osVersion) {
        downloading = true
        logger.info(`updating os ${this.osVersion} -> ${messages.osVersion}`)

        this.adapter.getOSImage(this.deviceType, this.osVersion).then(image => {
          downloading = false

          if (this.verifyHMAC && !this.verifyImageHMAC(messages.osImageHMAC, image)) {
            logger.warn('invalid app image HMAC, aborting update')
            return
          }

          this.updateOS(messages.osVersion)
        }).catch(e => {
          logger.error('failed to download app image:', e)
          downloading = false
        })
      }

      // Update App
      if (!downloading && messages.appVersion && messages.appVersion !== this.appVersion) {
        downloading = true
        logger.info(`updating ${this.appVersion} -> ${messages.appVersion}`)
        this.appVersion = messages.appVersion
        this.adapter.getAppImage(messages.appVersion).then(appZip => {
          downloading = false

          if (this.verifyHMAC && !this.verifyImageHMAC(messages.appImageHMAC, appZip)) {
            logger.warn('invalid app image HMAC, aborting update')
            return
          }

          this.launchApp(appZip)
        }).catch(e => {
          logger.error('failed to download app image:', e)
          downloading = false
        })

        return
      }

      // Send new stores to the app.
      if (messages.stores) {
        this.stores = messages.stores
        this.sendToApp('stores', { stores: messages.stores })
      }
    })

    setInterval(() => {
      this.sendHeartbeat('running')
    }, 3000)
  }
}

module.exports = Supervisor
