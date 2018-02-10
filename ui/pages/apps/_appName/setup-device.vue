<template>
  <dashboard-layout title="Setup a Device" :appName="appName">
    <card>
      <nuxt-link :to="{ name: 'apps-appName-install-agent', params: { name: appName } }">
        <h4>Run MakeStack agent on your Linux environment like Raspbian</h4>
      </nuxt-link>
    </card>
    <card>
      <nuxt-link :to="{ name: 'apps-appName-install-os', params: { name: appName } }">
        <h4>Install MakeStack Linux</h4>
        <p v-if="platform === 'desktop'">
          Use MakeStack Desktop app to install MakeStack Linux.
        </p>
      </nuxt-link>
    </card>
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
