<template>
  <dashboard-layout title="Setup a Device" :appName="appName">
    <header>
      <h2>Install Device Agent</h2>
    </header>
    <nuxt-link :to="{ name: 'apps-appName-devices', params: { name: appName } }">
      <i class="fas fa-chevron-left"></i>
      Return to Setup a Device
    </nuxt-link>
  </dashboard-layout>
</template>

<script>
import api from "~/assets/js/api"
import DashboardLayout from "~/components/dashboard-layout"

export default {
  components: { DashboardLayout },
  data() {
    return {
      appName: this.$route.params.appName
    }
  }
}
</script>
