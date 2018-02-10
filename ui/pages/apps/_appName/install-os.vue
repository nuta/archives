<template>
  <dashboard-layout title="Setup a Device" :appName="appName">
    <header>
      <h2>Install MakeStack Linux</h2>
    </header>
    <nuxt-link :to="{ name: 'apps-appName-devices', params: { name: appName } }">
      <i class="fas fa-chevron-left"></i>
      Return to Setup a Device
    </nuxt-link>

    <tabs>
      <tab name="Install">
        <section>
          <label>Device Name</label>
          <input type="text" v-model="deviceName" required="required" autofocus placeholder="Device Name">
        </section>

        <section>
          <label>Device Type</label>
          <select v-model="deviceType">
            <template v-for="type in availableDeviceTypes">
              <option :value="type.name" :key="type.name">{{ type.description }}</option>
            </template>
          </select>
        </section>

        <section>
          <label>OS</label>
          <select v-model="os">
            <template v-for="os in availableOSes">
              <option :value="os.name" :key="os.name">{{ os.description }}</option>
            </template>
          </select>
        </section>

        <section>
          <label>Network Adapter</label>
          <select v-model="adapter">
            <template v-for="adapter in availableAdapters">
              <option :value="adapter.name" :key="adapter.name">
                {{ adapter.description }}
              </option>
            </template>
          </select>
        </section>

        <section>
          <label>Install To</label>
          <select v-model="drive">
            <template v-for="drive in availableDrives">
              <option :value="drive.device" :key="drive.device">
                {{ drive.description }} ({{ drive.device }})
              </option>
            </template>
          </select>
        </section>

       <section>
            <button @click="install" class="primary">
              {{ installButtonMessage }}
            </button>
        </section>
      </tab>

      <tab name="Wi-Fi">
        <section>
          <label>Wi-Fi SSID</label>
          <input type="text" v-model="wifiSSID" placeholder="SSID">

          <label>Wi-Fi Password</label>
          <input type="password" v-model="wifiPassword" placeholder="Password (WPA2-PSK)">

          <label>Wi-Fi Country</label>
          <select v-model="wifiCountry">
            <template v-for="(name, code) in wifiCountries">
              <option :value="code" :key="name">{{ name }}</option>
            </template>
          </select>
        </section>
      </tab>
    </tabs>
  </dashboard-layout>
</template>

<script>
import api from "~/assets/js/api"
import DashboardLayout from "~/components/dashboard-layout"
import Tabs from "~/components/tabs"
import Tab from "~/components/fragments/tab"
import { ipcRenderer } from "electron"
import { setInterval, clearInterval } from 'timers';

export default {
  components: { DashboardLayout, Tabs, Tab },
  data() {
    return {
      appName: this.$route.params.appName,
      installButtonMessage: 'Install',
      deviceName: '',
      availableDeviceTypes: [
        { name: 'raspberrypi3', description: 'Raspberry Pi3' }
      ],
      deviceType: null,
      availableOSes: [
        { name: 'linux', description: 'MakeStack Linux' }
      ],
      os: null,
      drive: null,
      adapter: null,
      availableAdapters: [
        { name: 'http', description: 'HTTP/HTTPS (over Ethernet / Wi-Fi)' },
        { name: 'sakuraio', description: 'sakura.io' }
      ],
      availableDrives: [],
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
          if (this.availableDrives[0]) {
            this.drive = this.availableDrives[0].device
          }
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
    await this.refreshAvailableDrives()
    this.deviceType = this.availableDeviceTypes[0].name
    this.adapter = this.availableAdapters[0].name
    this.os = this.availableOSes[0].name
  },
  mounted() {
    this.refreshAvailableDrivesTimer = setInterval(async () => {
      this.refreshAvailableDrives()
    }, 3000)
  },
  beforeDestroy() {
    if (this.refreshAvailableDrivesTimer) {
      clearInterval(this.refreshAvailableDrivesTimer)
    }
  }
}
</script>
