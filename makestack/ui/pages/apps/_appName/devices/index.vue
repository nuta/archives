<template>
  <dashboard-layout title="Devices" :appName="appName">
    <guide-box v-if="devices.length === 0">
      <h1>You haven't set up any devices for {{ appName }} yet.</h1>
      <p>Let's install MakeStack Linux or Node.js CLI agent!</p>
      <nuxt-link :to="{ name: 'apps-appName-setup-device', params: { appName } }">
        <button class="primary">Set up Devices</button>
      </nuxt-link>
    </guide-box>

    <table v-else>
      <thead>
        <tr>
          <th>Name</th>
          <th>Registered at</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="device in devices" :key="device.name">
          <td>
            <nuxt-link :to="{ name: 'apps-appName-devices-deviceName', params: { appName, deviceName: device.name } }">
              {{ device.name }}
            </nuxt-link>
          </td>
          <td>{{ device.created_at }}</td>
        </tr>
      </tbody>
    </table>
  </dashboard-layout>
</template>

<script>
import api from "~/assets/js/api"
import DashboardLayout from "~/components/dashboard-layout"
import GuideBox from "~/components/guide-box"

export default {
  components: { DashboardLayout, GuideBox },
  data() {
    return {
      appName: this.$route.params.appName,
      devices: []
    }
  },
  async beforeMount() {
    this.devices = await api.getAppDevices(this.appName)
  }
}
</script>
