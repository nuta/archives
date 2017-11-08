const { fork, spawnSync } = require('child_process')
const fs = require('fs')
const os = require('os')
const crypto = require('crypto')
const path = require('path')
const HTTPAdapter = require('./adapters/http')
const SakuraioAdapter = require('./adapters/sakuraio')
const logger = require('./logger')

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
        this.adapter = new HTTPAdapter(deviceId, deviceSecret, adapter.url)
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
      this.adapter.send({
        state: 'ready',
        debugMode: this.debugMode,
        osVersion: this.osVersion,
        appVersion: 0,
        log: 'os updated'
      })
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

  sendHeartbeat(state) {
    logger.info(`heartbeating (state=running, os_ver="${this.osVersion}", ` +
      `app_ver="${this.appVersion}", debug=${this.debugMode})`)

    this.adapter.send({
      state,
      debugMode: this.debugMode,
      osVersion: this.osVersion,
      appVersion: this.appVersion,
      log: this.popLog()
    })
  }

  verifyImageHMAC(hmac, image) {
    if (!hmac) {
      return false
    }

    const hash = crypto.createHash('sha256')
    hash.update(image)
    const shasum = hash.digest('hex')

    const hmac2 = crypto.createHmac('sha256', this.deviceSecret)
    hmac2.update(shasum)

    return (hmac === hmac2.digest('hex'))
  }

  async start() {
    await this.adapter.connect()
    this.sendHeartbeat('ready')

    let downloading = false // XXX: use mutex or something better way
    this.adapter.onReceive(messages => {
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
        this.adapter.getAppImage(messages.appVersion).then((appZip) => {
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
