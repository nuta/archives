const { fork, spawnSync } = require('child_process')
const fs = require('fs')
const os = require('os')
const path = require('path')
const HTTPAdapter = require('./adapters/http')
const SakuraioAdapter = require('./adapters/sakuraio')
const logger = require('./logger')
const { I2CAPI } = require('app-runtime')

class Supervisor {
  constructor({ adapter, appDir, deviceType, osVersion, deviceId, deviceSecret, debugMode, appUID, appGID }) {
    this.app = null
    this.appDir = appDir
    this.osVersion = osVersion
    this.debugMode = debugMode
    this.appUID = parseInt(appUID) || undefined
    this.appGID = parseInt(appGID) || undefined
    this.deviceId = deviceId
    this.deviceType = deviceType
    this.device = new (require(`./devices/${deviceType}`))()
    this.appVersion = 0
    this.log = ''
    this.stores = {}
    this.adapterName = adapter.name
    switch (this.adapterName) {
      case 'http':
        this.adapter = new HTTPAdapter(deviceId, deviceSecret, adapter.url)
        break
      case 'sakuraio':
        this.adapter = new SakuraioAdapter(new I2CAPI())
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

  updateOS(version) {
    this.adapter.getOSImage(this.deviceType, version).then(image => {
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
    })
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

    this.app = fork('./start', {
      cwd: appDir,
      stdio: 'inherit',
      uid: this.appUID,
      gid: this.appGID
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

  async start() {
    await this.adapter.connect()
    logger.info(`heartbeating (state=booting, os_ver="${this.osVersion}", ` +
      `app_ver=0, debug=${this.debugMode})`)

    this.adapter.send({
      state: 'booting',
      debugMode: this.debugMode,
      osVersion: this.osVersion,
      appVersion: 0,
      log: ''
    })

    let downloading = false // XXX: use mutex or something better way
    this.adapter.onReceive(({ osUpdateRequest, appUpdateRequest, stores }) => {
      this.stores = stores

      if (!downloading && osUpdateRequest) {
        downloading = true
        this.updateOS(osUpdateRequest)
        downloading = false
      }

      if (!downloading && appUpdateRequest) {
        downloading = true
        logger.info(`updating ${this.appVersion} -> ${appUpdateRequest}`)
        this.appVersion = appUpdateRequest
        this.adapter.getAppImage(appUpdateRequest).then((appZip) => {
          downloading = false
          this.launchApp(appZip)
        }).catch(e => {
          logger.error('failed to download app image:', e)
          downloading = false
        })
      } else {
        this.sendToApp('stores', { stores })
      }
    })

    setInterval(() => {
      logger.info(`heartbeating (state=running, os_ver="${this.osVersion}", ` +
      `app_ver=${this.appVersion}, debug=${this.debugMode})`)

      this.adapter.send({
        state: 'running',
        debugMode: this.debugMode,
        osVersion: this.osVersion,
        appVersion: this.appVersion,
        log: this.popLog()
      })
    }, 3000)
  }
}

module.exports = Supervisor
