const { fork, spawnSync } = require('child_process')
const fs = require('fs')
const os = require('os')
const path = require('path')
const HTTPAdapter = require('./adapters/http')

class Supervisor {
  constructor(url, deviceType, osVersion, deviceId) {
    this.app = null
    this.osVersion = osVersion
    this.deviceId = deviceId
    this.deviceType = deviceType
    this.device = new (require(`./devices/${deviceType}`))()
    this.appVersion = 0
    this.log = ''
    this.stores = {}
    this.adapter = new HTTPAdapter(url, deviceId)
  }

  popLog() {
    const log = this.log
    this.log = ''
    return log
  }

  updateOS(version) {
    this.adapter.getOSImage(this.deviceType, version).then(image => {
      console.log('saving os image...')
      const tmpFilePath = path.join(os.tmpdir(), 'kernel.img')
      fs.writeFileSync(tmpFilePath, image)

      console.log('sending SIGTERM to the app...')
      if (this.app)
        this.app.kill()

      // Wait the app to exit.
      console.log('OS will be updated soon!')
      setTimeout(() => {
        console.log('updating os image...')
        this.device.updateOS(tmpFilePath)
        console.log('updateOS returned!')
        this.adapter.send({ state: 'ready', osVersion: this.osVersion, appVersion: 0, log: 'os updated' })
      }, 5000)
    })
  }

  launchApp(appZip) {
    const APP_ZIP_DIR = '/tmp/app.zip'
    const APP_DIR = '/tmp/app'

    fs.writeFileSync(APP_ZIP_DIR, appZip)
    spawnSync('rm', ['-r', APP_DIR])
    spawnSync('unzip', [APP_ZIP_DIR, '-d', APP_DIR], {
      stdio: 'inherit'
    })

    this.spawnApp(APP_DIR)
  }

  spawnApp(appDir) {
    if (this.app) {
      console.log('killing the app')
      this.app.kill()
    }

    this.app = fork('./start', {
      cwd: appDir,
      stdio: 'inherit'
    })
    this.sendToApp('initialize', { stores: this.stores })

    this.app.on('message', (data) => {
      console.log('message', data)
      switch (data.type) {
        case 'log':
          console.log('device', data.body)
          this.log += data.body
          break
        default:
          console.log('unknown message', data.type)
      }
    })

    this.app.on('exit', () => {
      this.app = null
      console.log('app exited')
    })
  }

  sendToApp(type, data) {
    if (!this.app) {
      // The app is being killed.
      return
    }

    this.app.send(Object.assign({ type }, data))
  }

  start() {
    this.adapter.send({ state: 'booting', osVersion: this.osVersion, appVersion: 0, log: '' })
    this.adapter.onReceive(({ osUpdateRequest, appUpdateRequest, stores }) => {
      this.stores = stores

      if (osUpdateRequest) {
        this.updateOS(osUpdateRequest)
      }

      if (appUpdateRequest) {
        console.log(`updating ${this.appVersion} -> ${appUpdateRequest}`)
        this.appVersion = appUpdateRequest
        this.adapter.getAppImage(appUpdateRequest).then((appZip) => {
          this.launchApp(appZip)
        })
      } else {
        this.sendToApp('stores', { stores })
      }
    })

    setInterval(() => {
      console.log('heartbeating...')
      this.adapter.send({
        state: 'running',
        osVersion: this.osVersion,
        appVersion: this.appVersion,
        log: this.popLog()
      })
    }, 3000)
  }
}

module.exports = Supervisor
