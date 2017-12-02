import { fork, spawnSync } from 'child_process';
import * as fs from 'fs';
import * as os from 'os';
import * as crypto from 'crypto';
import * as path from 'path';
import * as util from 'util';
import * as vm from 'vm';
import * as logger from './logger';
import { serialize, deserialize } from './smms';
import { verifyMessageHMAC, verifyImageHMAC } from './hmac';
import * as fsutils from './fsutils';
import * as unzip from './unzip';

interface SupervisorConstructorArgs {
  adapter: {
    name: string,
    url?: string
  };
  appDir: string;
  deviceType: string;
  osType: string;
  osVersion: string;
  deviceId: string;
  deviceSecret: string;
  debugMode: boolean;
  testMode: boolean;
  appUID?: number;
  appGID?: number;
  heartbeatInterval: number;
  runtimeModulePath: string;
}
export class Supervisor {
  app: any;
  appDir: string;
  currentAppDir: string;
  osType: string;
  osVersion: string;
  debugMode: boolean;
  testMode: boolean;
  appUID: number;
  appGID: number;
  heartbeatInterval: number;
  deviceId: string;
  deviceSecret: string;
  deviceType: string;
  device: any;
  appVersion: string;
  log: string;
  allLog: string;
  stores: any;
  adapterName: string;
  updateEnabled: boolean;
  downloading: boolean;
  adapter: any;
  verifyHMAC: boolean;
  includeHMAC: boolean;
  includeDeviceId: boolean;
  replEnabled: boolean;
  replVM?: any;
  rebooting: boolean;

  constructor(args: SupervisorConstructorArgs) {
    process.on('unhandledRejection', (reason, p) => {
      console.log('supervisor: unhandled rejection:', reason)
      console.log('supervisor: exiting...')
      process.exit(1)
    })

    this.app = null
    this.appDir = args.appDir
    this.currentAppDir = path.join(args.appDir, 'current')
    this.osType = args.osType
    this.osVersion = args.osVersion
    this.debugMode = args.debugMode
    this.testMode = args.testMode
    this.appUID = args.appUID
    this.appGID = args.appGID
    this.heartbeatInterval = args.heartbeatInterval || 15
    this.deviceId = args.deviceId
    this.deviceSecret = args.deviceSecret
    this.deviceType = args.deviceType
    const { Device } = require(`./devices/${args.deviceType}`)
    this.device = new Device()
    this.appVersion = 'X'
    this.log = ''
    this.allLog = ''
    this.stores = {}
    this.adapterName = args.adapter.name
    this.updateEnabled = true
    this.downloading = false
    this.replEnabled = args.debugMode;
    this.rebooting = false;

    process.env.RUNTIME_MODULE = args.runtimeModulePath || '@makestack/runtime'

    // Used in runtime. This must be set before require it.
    process.env.MAKESTACK_DEVICE_TYPE = args.deviceType

    if (this.replEnabled) {
      const { builtins } = require(args.runtimeModulePath);
      this.replVM = vm.createContext(builtins);
    }

    switch (this.adapterName) {
      case 'http': {
        const { HTTPAdapter } = require('./adapters/http_adapter')
        this.adapter = new HTTPAdapter(this.osType, this.deviceType, this.deviceId, args.adapter.url)
        this.verifyHMAC = true
        this.includeHMAC = true
        this.includeDeviceId = true
        break
      }
      case 'sakuraio': {
        const { SakuraIOAdapter } = require('./adapters/sakuraio_adapter')
        this.adapter = new SakuraIOAdapter()
        this.verifyHMAC = false
        this.includeHMAC = false
        this.includeDeviceId = false
        break
      }
      default:
        throw new Error(`unknown adapter \`${this.adapterName}'`)
    }
  }

  popLog() {
    const log = this.log
    this.log = ''
    return log
  }

  updateOS(image: Buffer) {
    logger.info('saving os image...')
    const tmpFilePath = path.join(os.tmpdir(), 'kernel.img')
    fs.writeFileSync(tmpFilePath, image)
    this.killApp()

    // Wait the app to exit.
    logger.warn('OS will be updated soon!')
    setTimeout(() => {
      logger.info('updating os image...')
      this.device.updateOS(tmpFilePath)
      logger.info('updateOS returned!')
      this.adapter.send(serialize({
        state: 'ready',
        deviceId: this.deviceId,
        debugMode: this.debugMode,
        osVersion: this.osVersion,
        appVersion: 0,
        log: 'os updated'
      }, {
        includeDeviceId: this.includeDeviceId,
        includeHMAC: this.includeHMAC,
        deviceSecret: this.deviceSecret
      }))
    }, 5000)
  }

  killApp() {
    if (this.app) {
      logger.warn('sending SIGTERM to the app...')
      this.app.kill()
    }
  }

  launchApp(appZip: Buffer) {
    fsutils.removeFiles(this.currentAppDir)
    unzip.extract(appZip, this.currentAppDir)
    this.spawnApp()
  }

  spawnApp() {
    if (this.app) {
      logger.info('killing the app')

      this.app.on('exit', () => {
        if (this.rebooting) {
          this.doReboot()
        }

        // This callback could called after 'error' event.
        // This guard is for preventing spawn an app twice.
        if (!this.app) {
          this.doSpawnApp()
        }
      })

      this.app.on('error', () => {
        // This callback could called after 'error' event.
        // This guard is for preventing spawn an app twice.
        if (!this.app) {
          this.doSpawnApp()
        }
      })

      this.app.kill()
    } else {
      this.doSpawnApp()
    }
  }

  doSpawnApp() {
    logger.info('starting an app')
    this.app = fork('./start', [], {
      cwd: this.currentAppDir,
      stdio: 'inherit',
      uid: this.appUID,
      gid: this.appGID,
      env: {
        MAKESTACK_DEVICE_TYPE: this.deviceType
      }
    } as any)
    this.sendToApp('initialize', { stores: this.stores })

    this.app.on('message', (data) => {
      logger.info('message', data)
      switch (data.type) {
        case 'log':
          logger.info('log:', data.body.trimRight())
          this.log += data.body.trimRight() + '\n'
          if (this.testMode) {
            this.allLog += data.body.trimRight() + '\n'
          }
          break
        case 'setUpdateEnabled':
          this.updateEnabled = (data.body !== false)
          break
        default:
          logger.info('unknown message', data.type)
      }
    })

    this.app.on('exit', () => {
      this.app = null
      logger.info('app exited')

      setTimeout(() => {
        if (!this.app) {
          logger.info('restarting app...')
          this.spawnApp()
        }
      }, 5000)
    })
  }

  waitForApp() {
    return new Promise((resolve, reject) => {
      if (!this.app) {
        resolve(this.allLog)
        return
      }

      this.app.on('exit',  () => { resolve(this.allLog) })
      this.app.on('error', () => { resolve(this.allLog) })
    })
  }

  sendToApp(type: string, data: Object) {
    if (!this.app) {
      // The app is being killed.
      return
    }

    this.app.send(Object.assign({ type }, data))
  }

  async sendHeartbeat(state) {
    if (this.downloading) {
      return
    }

    logger.info(`heartbeating (state=running, os_ver="${this.osVersion}", ` +
      `app_ver="${this.appVersion}", debug=${this.debugMode})`)

    await this.adapter.send(serialize({
      state,
      deviceId: this.deviceId,
      debugMode: this.debugMode,
      osVersion: this.osVersion,
      appVersion: this.appVersion,
      log: this.popLog()
    }, {
      includeDeviceId: this.includeDeviceId,
      includeHMAC: this.includeHMAC,
      deviceSecret: this.deviceSecret
    }))
  }

  async handleUpdateAppMessage(appVersion: string, appImageHMAC: string) {
    logger.info(`updating ${this.appVersion} -> ${appVersion}`)
    this.appVersion = appVersion

    let appZip
    this.downloading = true
    try {
      appZip = await this.adapter.getAppImage(appVersion)
    } catch(e) {
      logger.error('failed to download app image:', e)
      this.downloading = false
      return
    } finally {
      this.downloading = false
    }

    if (this.verifyHMAC && !verifyImageHMAC(this.deviceSecret, appImageHMAC, appZip)) {
      logger.warn('invalid app image HMAC, aborting update')
      return
    }

    this.launchApp(appZip)
  }

  handleUpdateOSMessage(osVersion, osImageHMAC) {
    this.downloading = true
    logger.info(`updating os ${this.osVersion} -> ${osVersion}`)

    this.adapter.getOSImage(this.deviceType, this.osVersion).then(image => {
      this.downloading = false

      if (this.verifyHMAC && !verifyImageHMAC(this.deviceSecret, osImageHMAC, image)) {
        logger.warn('invalid os image HMAC, aborting update')
        return
      }

      this.updateOS(osVersion)
    }).catch(e => {
      logger.error('failed to download app image:', e)
      this.downloading = false
    })
  }

  handleREPLCommand(commandId: string, code: string): void {
    logger.debug(`REPL: eval id=${commandId} code='${code}'`)
    const result = JSON.stringify(
      util.inspect(
        vm.runInContext(code, this.replVM)
      )
    )

    // Event.publish()
    this.log += `>${commandId} __repl__ ${result}\n`
  }

  tryBuiltinCommand(name: string, key: string, callback: (string) => void): boolean {
    const regex = new RegExp('^<([0-9]+) __' + name + '__$')
    const m = regex.exec(key)
    if (m) {
      callback(m[1])
      return true
    }

    return false
  }

  reboot() {
    this.rebooting = true
    if (this.app) {
      setTimeout(() => {
        logger.warn('app does not exit, forcing reboot...')
        this.doReboot()
      }, 10 * 1000)

      this.killApp()
    } else {
      this.doReboot()
    }
  }

  doReboot() {
    // /init script reboots the system if Supervisor exit with 0.
    logger.info('Received a reboot command. Exiting with 0...')
    process.exit(0)
  }

  handleStoreMessage(stores: [string]) {
    const storesToApp = {}
    for (const key in stores) {
      const isBuiltinCommand = [
        this.replEnabled &&
          this.tryBuiltinCommand('repl', key, id => this.handleREPLCommand(id, stores[key])),
        this.tryBuiltinCommand('reboot', key, id => this.reboot())
      ].some(x => x)

      if (!isBuiltinCommand) {
        storesToApp[key] = stores[key]
      }
    }

    this.stores = storesToApp
    this.sendToApp('stores', { storesToApp })
  }

  isOSUpdateRequired(messages): boolean {
    return this.updateEnabled && this.osVersion && !this.downloading && messages.osVersion && messages.osVersion !== this.osVersion
  }

  isAppUpdateRequired(messages): boolean {
    return this.updateEnabled && !this.downloading && messages.appVersion && messages.appVersion !== this.appVersion
  }

  async onSMMSReceive(payload) {
    const [messages, hmacProtectedEnd] = deserialize(payload)
    const hmacProtectedPayload = payload.slice(0, hmacProtectedEnd)

    if (this.verifyHMAC && !verifyMessageHMAC(this.deviceSecret, hmacProtectedPayload, messages.hmac, messages.timestamp)) {
      logger.warn('invalid message hmac, discarding...')
      return
    }

    // Update OS.
    if (this.isOSUpdateRequired(messages)) {
      this.handleUpdateOSMessage(messages.osVersion, messages.osImageHMAC)
    }

    // Update App
    if (this.isAppUpdateRequired(messages)) {
      await this.handleUpdateAppMessage(messages.appVersion, messages.appImageHMAC)
      return
    }

    // Send new stores to the app.
    if (messages.stores) {
      this.handleStoreMessage(messages.stores)
    }
  }


  async start() {
    this.adapter.onReceive(payload => this.onSMMSReceive(payload))
    await this.adapter.connect()
    await this.sendHeartbeat('ready')

    setInterval(() => {
      this.sendHeartbeat('running')
    }, this.heartbeatInterval * 1000)
  }
}
