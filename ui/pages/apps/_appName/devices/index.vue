<template>
  <dashboard-layout title="Devices" :appName="appName">
    <card v-for="device in devices" :key="device.name"
     :title="device.name" :subtitle="`Registered at ${device.created_at}`"
     clickable="true" @click="onSelect(device)">
    </card>
  </dashboard-layout>
</template>

<script>
import api from "~/assets/js/api"
import DashboardLayout from "~/components/dashboard-layout"
import Card from "~/components/card"

export default {
  components: { DashboardLayout, Card },
  data() {
    return {
      appName: this.$route.params.appName,
      devices: []
    }
  },
  methods: {
    onSelect(device) {
      this.$router.push({
        name: 'apps-appName-devices-deviceName',
        params: {
          name: this.appName,
          deviceName: device.name
          }
      })
    }
  },
  async beforeMount() {
    this.devices = await api.getAppDevices(this.appName)
  }
}
</script>
