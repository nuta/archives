<template>
<app-layout path="appLog" :app-name="appName">
  <log-viewer :lines="log"></log-viewer>
</app-layout>
</template>

<script>
import api from "js/api"
import LogViewer from "components/log-viewer"
import AppLayout from "layouts/app"

export default {
  components: { AppLayout, LogViewer },
  data() {
    return {
      appName: app.$router.currentRoute.params.appName,
      log: [],
      timer: undefined,
      lastFetchedAt: null
    };
  },
  methods: {
    async fetchNewLines() {
      const lines = (await api.getAppLog(this.appName, this.lastFetchedAt)).lines || []
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
