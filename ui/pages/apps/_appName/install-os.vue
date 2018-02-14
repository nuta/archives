<template>
  <dashboard-layout title="Setup a Device" :appName="appName" inverted-bg="true">
    <guide-box v-if="platform !== 'desktop'">
      <h1>Installing MakeStack Linux on your devices</h1>
      <p>Use desktop app to install MakeStack Linux to a SD card:</p>
      <a href="https://makestack.org">
        <button class="primary">Download Desktop App</button>
      </a>
    </guide-box>
    <div v-else>
      <header>
        <h2>Install MakeStack Linux</h2>
      </header>
      <nuxt-link :to="{ name: 'apps-appName-devices', params: { name: appName } }">
        <i class="fas fa-chevron-left"></i>
        Return to Setup a Device
      </nuxt-link>

      <tabs>
        <form class="small" @submit.prevent="void(0)">
          <tab name="Install">
            <div class="field">
              <label>Device Name</label>
              <input type="text" v-model="deviceName" required="required" autofocus placeholder="Device Name">
            </div>

            <div class="field">
              <label>Device Type</label>
              <select v-model="deviceType">
                <template v-for="type in availableDeviceTypes">
                  <option :value="type.name" :key="type.name">{{ type.description }}</option>
                </template>
              </select>
            </div>

            <div class="field">
              <label>Install To</label>
              <p v-if="availableDrives.length === 0">
                No drives detected. Insert a SD Card!
              </p>
              <select v-model="drive" v-else>
                <template v-for="drive in availableDrives">
                  <option :value="drive.device" :key="drive.device">
                    {{ drive.description }} ({{ drive.device }})
                  </option>
                </template>
              </select>
            </div>

            <div class="field">
              <button @click="install" class="primary">
                  {{ installButtonMessage }}
              </button>
            </div>
          </tab>

          <tab name="Network">
            <div class="field">
              <label>Network Adapter</label>
              <select v-model="adapter">
                <template v-for="adapter in availableAdapters">
                  <option :value="adapter.name" :key="adapter.name">
                    {{ adapter.description }}
                  </option>
                </template>
              </select>
            </div>
          </tab>

          <tab name="Wi-Fi">
            <div class="field">
              <label>Wi-Fi SSID</label>
              <input type="text" v-model="wifiSSID" placeholder="SSID">
            </div>

            <div class="field">
              <label>Wi-Fi Password</label>
              <input type="password" v-model="wifiPassword" placeholder="Password (WPA2-PSK)">
            </div>

            <div class="field">
              <label>Wi-Fi Country</label>
              <select v-model="wifiCountry">
                <template v-for="(name, code) in wifiCountries">
                  <option :value="code" :key="name">{{ name }}</option>
                </template>
              </select>
            </div>
          </tab>
        </form>
      </tabs>
    </div>
  </dashboard-layout>
</template>

<script>
import api from "~/assets/js/api"
import DashboardLayout from "~/components/dashboard-layout"
import GuideBox from "~/components/guide-box"
import Tabs from "~/components/tabs"
import Tab from "~/components/fragments/tab"
import { setInterval, clearInterval } from 'timers';

export default {
  components: { DashboardLayout, Tabs, Tab, GuideBox },
  data() {
    return {
      platform: PLATFORM,
      appName: this.$route.params.appName,
      installButtonMessage: 'Install',
      deviceName: '',
      availableDeviceTypes: [
        { name: 'raspberrypi3', description: 'Raspberry Pi3 (MakeStack Linux)' }
      ],
      deviceType: null,
      os: 'linux',
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
      ipcRenderer.on('installProgress', (event, stage, state) => {
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

      ipcRenderer.on('installError', (event, message) => {
        this.installButtonMessage = 'Install'
        alert(message)
      })

      ipcRenderer.send('install', {
        deviceName: this.deviceName,
        deviceType: this.deviceType,
        os: this.os,
        app: this.appName,
        adapter: this.adapter,
        drive: this.drive,
        wifiSSID: this.wifiSSID,
        wifiPassword: this.wifiPassword,
        wifiCountry: this.wifiCountry
      })
    }
  },
  async beforeMount() {
    if (this.platform === 'desktop') {
      await this.refreshAvailableDrives()
      this.deviceType = this.availableDeviceTypes[0].name
      this.adapter = this.availableAdapters[0].name
    }
  },
  mounted() {
    if (this.platform === 'desktop') {
      this.refreshAvailableDrivesTimer = setInterval(async () => {
        this.refreshAvailableDrives()
      }, 3000)
    }
  },
  beforeDestroy() {
    if (this.refreshAvailableDrivesTimer) {
      clearInterval(this.refreshAvailableDrivesTimer)
    }
  }
}
</script>
