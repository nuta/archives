<template>
  <dashboard-layout title="Setup a Device" :appName="appName">
    <div class="grid">
      <card title="Run agent on Linux/macOS"
        subtitle="Run MakeStack agent on your Linux environment like Raspbian."
       :to="{ name: 'apps-appName-install-agent', params: { name: appName } }">
      </card>
      <card title="Install MakeStack Linux"
        subtitle="Install a minimalized Linux distribution made for MakeStack apps."
       :to="{ name: 'apps-appName-install-os', params: { name: appName } }">
      </card>
    </div>
  </dashboard-layout>
</template>

<script>
import Card from "~/components/card"
import DashboardLayout from "~/components/dashboard-layout"

export default {
  components: { DashboardLayout, Card },
  data() {
    return {
      platform: PLATFORM,
      appName: this.$route.params.appName
    }
  }
}
</script>
