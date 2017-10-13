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
      <label>OS</label>
      <select v-model="os">
        <template v-for="name in availableOSes">
          <option :value="name">{{ name }}</option>
        </template>
      </select>
    </section>

    <section>
      <label>Install To</label>
      <select v-model="drive">
        <template v-for="drive in availableDrives">
          <option :value="drive.device">{{ drive.description }} ({{ drive.device }})</option>
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
      <b>Overwrite</b>
      <input type="checkbox" :value="ignoreDuplication">
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
const fetch = require('node-fetch')
const drivelist = require('drivelist')
const makestack = require('makestack')

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
      availableOSes: [ 'linux' ],
      os: '',
      drive: "",
      adapter: '',
      availableAdapters: [
        { name: 'ethernet', description: 'Ethernet (DHCP)' }
      ],
      availableDrives: [],
      percentage: 0,
      ignoreDuplication: false
    }
  },
  computed: {
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
    async refreshAvailableDrives() {
      this.availableDrives = await makestack.drive.getAvailableDrives()
    },
    install() {
      const flashCommand = [process.argv0, path.resolve(__dirname, 'flasher.js')]
      makestack.install(
        this.deviceName, this.deviceType, this.os,
        this.adapter, this.drive, this.ignoreDuplication,
        flashCommand, (stage, state) => {

        switch (stage) {
          case 'look-for-device':
            this.message = '(1/5) Looking for the drive'
            break
          case 'register':
            this.message = '(2/5) Registering the device'
            break
          case 'download':
            this.message = '(3/5) Downloading the disk image'
            break
          case 'config':
            this.message = '(4/5) Writing config'
            break
          case 'flash':
            this.message = '(5/5) Flashing'
            break
          case 'flashing':
            const message = {write: 'Writing', check: 'Verifying' }[state.type]
            this.message = `${message}...(${state.percentage}%)`
            break
        }
      })

      process.env.ELECTRON_RUN_AS_NODE = undefined
    }
  },
  async beforeMount() {
    if (!api.isLoggedIn())
      this.$router.push({ name: 'login' })

    await this.refreshAvailableDrives()
    this.deviceType = this.availableDeviceTypes[0] || "";
    this.drive = this.availableDrives[0] || "";
    this.adapter = this.availableAdapters[0].name || ''

    // DEBUG
    this.deviceName = 'abc'
    this.drive = '/dev/disk2'
    this.os = 'linux'
    this.ignoreDuplication = true
  }
}
</script>

<style lang="scss" scoped>
section {
  margin-top: 20px;
}
</style>
