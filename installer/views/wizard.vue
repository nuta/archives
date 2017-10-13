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
import { ipcRenderer } from 'electron'

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
      this.availableDrives = ipcRenderer.sendSync('getAvailableDrives')
    },
    install() {
      ipcRenderer.on('progress', (event, arg) => {
        console.log(arg)
      })

      ipcRenderer.send('install', {
        deviceName: this.deviceName,
        deviceType: this.deviceType,
        os: this.os,
        adapter: this.adapter,
        drive: this.drive,
        ignoreDuplication: this.ignoreDuplication
      })
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
