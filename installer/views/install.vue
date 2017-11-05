<template>
<div class="wallpaper">
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
        <label><input type="checkbox" :value="ignoreDuplication">Overwrite</label>
      </section>

      <section>
        <div class="install-button-wrapper">
          <input type="submit" :value="installButtonMessage">
        </div>
      </section>
    </form>
  </main>
</div>
</template>

<script>
import api from 'renderer/api'
import { ipcRenderer } from 'electron'

export default {
  components: { },
  data() {
    return {
      installButtonMessage: 'Install',
      deviceName: '',
      availableDeviceTypes: [ 'raspberrypi3', 'mock' ],
      deviceType: '',
      availableOSes: [ 'linux' ],
      os: '',
      drive: '',
      adapter: '',
      availableAdapters: [
        { name: 'ethernet', description: 'Ethernet (DHCP)' }
      ],
      availableDrives: [],
      percentage: 0,
      ignoreDuplication: false
    }
  },
  methods: {
    async refreshAvailableDrives() {
      this.availableDrives = ipcRenderer.sendSync('getAvailableDrives')
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
    this.deviceType = this.availableDeviceTypes[0] || ''
    this.drive = this.availableDrives[0] || ''
    this.adapter = this.availableAdapters[0].name || ''

    // DEBUG
    this.deviceName = 'abc'
    this.drive = '/dev/disk2'
    this.os = 'linux'
    this.deviceType = 'mock'
    this.ignoreDuplication = true
  }
}
</script>

<style lang="scss" scoped>
header {
  background: linear-gradient(to left, #7474bf, #348ac7);

  h1 {
    text-align: center;
    font-weight: 200;
    font-size: 25px;
    padding: 35px 5px;
    color: #ffffff;
    margin: 0;
  }
}

.wallpaper {
  width: 100vw;
  height: 100vh;
  background: #f6f6f6;
}

main {
  margin-top: 10px;
  margin-left: auto;
  margin-right: auto;
  width: 300px;
}

section {
  &:not(:first-child) {
    margin-top: 15px;
  }

  label {
    display: block;
    font-size: 15px;
    margin-bottom: 5px;
  }

  input[type=text], input[type=password], select {
    width: 100%;
    font-size: 15px;
  }

  input[type=checkbox] {
    margin-right: 10px;
  }

  .install-button-wrapper {
    text-align: center;
  }

  input[type=submit] {
    width: 300px;
    margin-top: 25px;
    border-radius: 5px;
    padding: 10px 25px;
    font-size: 15px;
    font-weight: 600;
    background: #1e87f0;
    color: #ffffff;
    border: 1px solid transparent;

    &:active {
      background: #0e6dcd;
    }

    &:hover {
      background: #0f7ae5;
      cursor: pointer;
    }
  }
}
</style>
