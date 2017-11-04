<template>
<device-layout path="deviceLog" :device-name="deviceName">
  <log-viewer :lines="log"></log-viewer>
</device-layout>
</template>

<script>
import api from "js/api"
import LogViewer from "components/log-viewer"
import DeviceLayout from "layouts/device"

export default {
  components: { DeviceLayout, LogViewer },
  data() {
    return {
      deviceName: app.$router.currentRoute.params.deviceName,
      log: [],
      timer: undefined,
      lastFetchedAt: null
    }
  },
  methods: {
    async fetchNewLines() {
      const lines = (await api.getDeviceLog(this.deviceName, this.lastFetchedAt)).lines || []
      this.lastFetchedAt = new Date()
      return lines
    }
  },
  async beforeMount() {
    this.log = await this.fetchNewLines()
    this.$Progress.finish()

    this.timer = setInterval(async () => {
      this.$Progress.start()
      this.log= this.log.concat(await this.fetchNewLines())
      this.$Progress.finish()
    }, 5000)
  },
  beforeRouteLeave (to, from, next) {
    clearInterval(this.timer)
    next()
  }
};
</script>
