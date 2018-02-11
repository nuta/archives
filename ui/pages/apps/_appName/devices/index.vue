<template>
  <dashboard-layout title="Devices" :appName="appName">
    <guide-box v-if="devices.length === 0">
      <h1>You haven't set up any devices for {{ appName }} yet.</h1>
      <p>Let's install MakeStack Linux or Node.js CLI agent!</p>
      <nuxt-link :to="{ name: 'apps-appName', params: { appName } }">
        <button class="primary">Set up Devices</button>
      </nuxt-link>
    </guide-box>

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
import GuideBox from "~/components/guide-box"

export default {
  components: { DashboardLayout, Card, GuideBox },
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
