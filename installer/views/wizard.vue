<template>
<div>
  <h1>MakeStack Installer</h1>
  <form v-on:submit.prevent="install">
    <section>
      <label>Device Name</label>
      <input type="text" v-model="deviceName" required="required" autofocus placeholder="Device Name">
    </section>
    
    <section>
      <label>Device Type</label>
      <select v-model="deviceType">
        <template v-for="type in availableDeviceTypes">
          <option :value="type">{{type}}</option>
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
      <b>{{ message }}</b>
      <input type="submit" :value="installButtonMsg">    
    </section>
  </form>
</div>
</template>

<script>
const fs = require('fs');
const { spawn } = require('child_process');

export default {
  components: { },
  data() {
    return {
      message: "",
      deviceName: "",
      availableDeviceTypes: ["raspberrypi3"],
      deviceType: "",
      devFile: "",
      availableDevFiles: [],
      percentage: 0,
      installing: false
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
    refreshAvailableDevFiles() {      
      this.availableDevFiles = ['/tmp/foo']
    },
    install() {
      this.installing = true;
      
      // TODO: download and cache disk images
      const diskImagePath = '/tmp/sdcard.img'
      const diskImageSize = fs.statSync(diskImagePath).size

      // TODO: support Linux (GNU one)
      const args = ['bs=4m', `if=${diskImagePath}`, `of=${this.devFile}`]
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
    this.devFile    = this.availableDevFiles[0] || "";
  }
}
</script>

<style lang="scss" scoped>
section {
  margin-top: 20px;
}
</style>
