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

      <section v-if="adapter == 'http'">
        <details>
          <summary>Wi-Fi (Optional)</summary>
          <div>
            <label>Wi-Fi SSID</label>
            <input type="text" v-model="wifiSSID" placeholder="SSID">

            <label>Wi-Fi Password</label>
            <input type="password" v-model="wifiPassword" placeholder="Password (WPA2-PSK)">

            <label>Wi-Fi Country</label>
            <select v-model="wifiCountry">
              <template v-for="(name, code) in wifiCountries">
                <option :value="code">{{ name }}</option>
              </template>
            </select>
          </div>
        </details>
      </section>

      <section>
        <label><input type="checkbox" v-model="ignoreDuplication">Ignore a device name duplication.</label>
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
        { name: 'http', description: 'HTTP/HTTPS (over Ethernet / Wi-Fi)' },
        { name: 'sakuraio', description: 'sakura.io' }
      ],
      availableDrives: [],
      ignoreDuplication: false,
      wifiSSID: '',
      wifiPassword: '',
      wifiCountry: 'US',
      /* TODO: add all countries */
      wifiCountries: {
        'US': 'United States of America',
        'UK': 'United Kingdom',
        'JP': 'Japan',
        'CN': 'China',
        'IN': 'India',
        'ID': 'Indonesia',
        'IL': 'Israel',
        'DE': 'Germany',
        'FR': 'France',
        'RU': 'Russia'
      }
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
      ipcRenderer.on('progress', (event, stage, state) => {
        switch (stage) {
          case 'look-for-drive':
            this.installButtonMessage = '(1/5) Looking for the drive'
            break
          case 'register':
            this.installButtonMessage = '(2/5) Registering the device'
            break
          case 'download':
            this.installButtonMessage = '(3/5) Downloading the disk image'
            break
          case 'config':
            this.installButtonMessage = '(4/5) Writing config'
            break
          case 'flash':
            this.installButtonMessage = '(5/5) Flashing'
            break
          case 'flashing':
            switch (state.type) {
              case 'write':
                this.installButtonMessage = `Flashing (${Math.floor(state.percentage)}%)`
                break;
              case 'check':
                this.installButtonMessage = `Verifying (${Math.floor(state.percentage)}%)`
                break;
            }
            break
          case 'success':
            new Notification('Successfully installed MakeStack', {
              body: `${this.deviceName} is now ready to use. Have fun!`
            })

            this.installButtonMessage = 'Done!'
            setTimeout(() => {
              this.installButtonMessage = 'Install'
            }, 3000)
            break
        }
      })

      ipcRenderer.on('error', (event, message) => {
        this.installButtonMessage = 'Install'
        alert(message)
      })

      ipcRenderer.send('install', {
        deviceName: this.deviceName,
        deviceType: this.deviceType,
        os: this.os,
        adapter: this.adapter,
        drive: this.drive,
        ignoreDuplication: this.ignoreDuplication,
        wifiSSID: this.wifiSSID,
        wifiPassword: this.wifiPassword,
        wifiCountry: this.wifiCountry
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
