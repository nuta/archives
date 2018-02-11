<template>
  <div></div>
</template>

<script>
import api from "~/assets/js/api"
import { getLastUsedApp } from "~/assets/js/preferences"

export default {
  async beforeCreate() {
    let appName = getLastUsedApp()
    if (!appName) {
      const app = (await api.getApps())[0]
      if (!app) {
        this.$router.push({ name: 'createApp' })
      }
      appName = app.name
    }

    this.$router.push({ name: 'apps-appName', params: { appName: appName } })
  }
}
</script>
