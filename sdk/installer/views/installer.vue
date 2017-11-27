<template>
<main-layout>
  <header>
    <h1>MakeStack Installer</h1>
  </header>

  <main>
    <form @submit.prevent="install">
      <section>
        <label>Device Name</label>
        <input type="text" v-model="deviceName" required="required" autofocus placeholder="Device Name">
      </section>

      <section>
        <label>Device Type</label>
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
        <label><input type="checkbox" :value="ignoreDuplication">Ignore a device name duplication.</label>
      </section>

      <section>
          <input type="submit" :value="installButtonMessage">
      </section>
    </form>
  </main>
</main-layout>
</template>

<script>
import api from 'renderer/api'
import MainLayout from 'layouts/main'
import { ipcRenderer } from 'electron'

export default {
  components: { MainLayout },
  data() {
    return {
      installButtonMessage: 'Install',
      deviceName: '',
      availableDeviceTypes: [ 'raspberrypi3' ],
      deviceType: null,
      availableOSes: [ 'linux' ],
      os: null,
      drive: null,
      adapter: null,
      availableAdapters: [
        { name: 'ethernet', description: 'Ethernet (DHCP)' }
      ],
      availableDrives: [],
      percentage: 0,
      ignoreDuplication: false
    }
  },
  methods: {
    refreshAvailableDrives() {
      this.availableDrives = ipcRenderer.sendSync('getAvailableDrives')
      if (!this.drive) {
        this.$nextTick(() => {
          this.drive = this.availableDrives[0].device
        })
      }
    },
    install() {
      const component = this
      ipcRenderer.on('progress', (event, stage) => {
        switch (stage) {
          case 'look-for-drive':
            component.installButtonMessage = '(1/5) Looking for the drive'
            break
          case 'register':
            component.installButtonMessage = '(2/5) Registering the device'
            break
          case 'download':
            component.installButtonMessage = 'Downloading the disk image'
            break
          case 'config':
            component.installButtonMessage = '(4/5) Writing config'
            break
          case 'flash':
            component.installButtonMessage = '(5/5) Flashing'
            break
          case 'success':
            new Notification('Successfully installed MakeStack', {
              body: `${component.deviceName} is now ready to use. Have fun!`
            })

            component.installButtonMessage = 'Install'
            break
        }
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
    this.deviceType = this.availableDeviceTypes[0]
    this.adapter = this.availableAdapters[0].name
    this.os = this.availableOSes[0]
  },
  mounted() {
    setInterval(async () => {
      this.refreshAvailableDrives()
    }, 3000)
  }
}
</script>

<style lang="scss" scoped>
</style>
