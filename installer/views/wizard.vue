<template>
<div>
  <h1>MakeStack Installer</h1>
  <form v-on:submit.prevent="install">
    <section>
      <label>Device Name</label>
      <input type="text" v-model="deviceName" required="required" autofocus placeholder="Device Name">
    </section>

    <section>
      <label>Type</label>
      <select v-model="deviceType">
        <template v-for="type in availableDeviceTypes">
          <option :value="type">{{ type }}</option>
        </template>
      </select>
    </section>

    <section>
      <label>Image</label>
      <select v-model="osImageURL">
        <template v-for="image in availableOSImages">
          <option :value="image.url">{{ image.name }}</option>
        </template>
      </select>
    </section>

    <section>
      <label>Install To</label>
      <select v-model="devFile">
        <template v-for="path in availableDevFiles">
          <option :value="path">{{ path }}</option>
        </template>
      </select>
    </section>

    <section>
      <label>Network Adapter</label>
      <select v-model="adapter">
        <template v-for="adapter in availableAdapters">
          <option :value="adapter.name">{{ adapter.description }}</option>
        </template>
      </select>
    </section>

    <section>
      <b>{{ message }}</b>
      <input type="submit" :value="installButtonMsg">
    </section>
  </form>
</div>
</template>

<script>
import api from 'renderer/api'
const os = require('os')
const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');
const fetch = require('node-fetch')
const uuid = require('uuid/v4')

function replaceBuffer(buf, value, id) {
  const needle = `_____REPLACE_ME_MAKESTACK_CONFIG_${id}_____`

  const index = buf.indexOf(Buffer.from(needle))
  if (index == -1)
    throw `replaceBuffer: failed to replace ${id}`

  let paddedValue = Buffer.alloc(needle.length, ' ')
  let valueBuf = Buffer.from(value)
  valueBuf.copy(paddedValue)
  paddedValue.copy(buf, index)
  return buf
}

export default {
  components: { },
  data() {
    return {
      message: "",
      deviceName: "",
      availableDeviceTypes: [ 'raspberrypi3' ],
      deviceType: "",
      osImageURL: '',
      devFile: "",
      adapter: '',
      availableAdapters: [
        { name: 'ethernet', description: 'Ethernet (DHCP)' }
      ],
      availableDevFiles: [],
      percentage: 0,
      installing: false
    }
  },
  computed: {
    availableOSImages() {
      switch (this.deviceType) {
        case 'raspberrypi3':
          return [
            {
              name: 'MakeStack Linux version 0.0.1',
              url: 'https://www.coins.tsukuba.ac.jp/~s1311386/kernel.img'
             }
          ]
      }
    },
    installButtonMsg() {
      if (this.installing) {
        return `Installing (${this.percentage}%)`
      } else {
        if (this.deviceName.length > 0)
          return `Install MakeStack to ${this.deviceName}`
        else
          return "Install"
      }
    }
  },
  methods: {
    refreshAvailableDevFiles() {
      this.availableDevFiles = ['/tmp/foo']
    },
    async install() {
      this.installing = true;

      //
      //  Stage 1: Register the device
      //
      let device = (await api.registerDevice(this.deviceName, this.deviceType)).json

      //
      //  Stage 2: Download the OS image
      //
      const basename = path.basename(this.osImageURL)
      const originalDiskImageFile = path.join(process.env.HOME, `.makestack/caches/${basename}`)
      try {
        fs.mkdirSync(path.join(process.env.HOME, '.makestack'))
      } catch (e) {
        // ignore
      }

      try {
        fs.mkdirSync(path.join(process.env.HOME, '.makestack/caches'))
      } catch (e) {
        // ignore
      }

      fs.writeFileSync(originalDiskImageFile, await (await fetch(this.osImageURL)).buffer())

      //
      //  Stage 3: Overwrite config.sh
      //
      const diskImageFile = path.join(os.tmpdir(), uuid() + '.img')

      // TODO: What if the image is large?
      let image = fs.readFileSync(originalDiskImageFile)
      let device = {device_id: 'asd'}
      image = replaceBuffer(image, device.device_id, 'DEVICE_ID')
      image = replaceBuffer(image, device.device_secret, 'DEVICE_SECRET')
      image = replaceBuffer(image, api.serverURL, 'SERVER_URL_abcdefghijklmnopqrstuvwxyz1234567890')
      image = replaceBuffer(image, device.adapter, 'ADAPTER')
      fs.writeFileSync(diskImageFile, image)

      //
      //  Stage 4: Flash!
      //
      const diskImageSize = fs.statSync(diskImageFile).size

      // TODO: support Linux (GNU one)
      const args = ['bs=4m', `if=${diskImageFile}`, `of=${this.devFile}`]
      console.log(args)
      const dd = spawn('/bin/dd', args)

      const checkProgress = setInterval(() => {
        dd.kill('SIGINFO')
      }, 100)

      dd.stdout.on('data', data => {
        console.log(`${data}`)
      })

      dd.stderr.on('data', data => {
        const str = data.toString()
        const match = /[^\n]*\n(\d+) bytes/.exec(str)
        const transferred = match[1]

        this.percentage = Math.floor(transferred / diskImageSize * 100)
        console.error(`${str}`)
      })

      dd.on('close', code => {
        clearInterval(checkProgress);
        if (code != 0) {
          this.message = 'something went wrong :('
        } else {
          this.message = 'done!'
        }

        this.installing = false;
      })
    }
  },
  beforeMount() {
    if (!api.isLoggedIn())
      this.$router.push({ name: 'login' })

    this.refreshAvailableDevFiles();
    this.deviceType = this.availableDeviceTypes[0] || "";
    this.devFile = this.availableDevFiles[0] || "";
    this.adapter = this.availableAdapters[0].name || ''

    // DEBUG
    this.deviceName = 'abc'
    this.osImageURL = 'https://www.coins.tsukuba.ac.jp/~s1311386/kernel.img'
  }
}
</script>

<style lang="scss" scoped>
section {
  margin-top: 20px;
}
</style>
