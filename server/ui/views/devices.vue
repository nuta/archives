<template>
<dashboard-layout title="Devices">
  <remote-content :loading="loading" :content="devices">
    <p slot="welcome-title">No devices yet!</p>

    <table slot="content" class="uk-table uk-table-divider">
      <thead>
        <tr>
          <th>Name</th>
          <th>Type</th>
          <td>App</td>
          <th>Tag</th>
          <td>Registered at</td>
          <th></th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="device in devices">
          <td>
            <router-link :to="{ name: 'deviceSettings', params: { deviceName: device.name }}">
              {{ device.name }}
            </router-link>
          </td>
          <td>{{ device.device_type }}</td>
          <td>
            <template v-if="device.app">
              <router-link :to="{ name: 'code', params: { appName: device.app }}">
                <span uk-icon="icon: forward"></span>
                {{ device.app }}
              </router-link>
            </template>
            <template v-else>
              (not associated)
            </template>
          </td>
          <td>{{ device.tag || "(unspecified)" }}</td>
          <td>{{ device.created_at | strftime }}</td>
          <td class="actions">
            <router-link :to="{ name: 'deviceSettings', params: { deviceName: device.name }}">
              <span uk-icon="icon: forward"></span>
              Open Dashboard
            </router-link>
          </td>
        </tr>
      </tbody>
    </table>
  </remote-content>
</dashboard-layout>
</template>

<script>
import api from "js/api";
import { strftime } from "js/filters";
import RemoteContent from "components/remote-content";
import DashboardLayout from "layouts/dashboard";

export default {
  components: { DashboardLayout, RemoteContent },
  filters: { strftime },
  data() {
    return {
      loading: true,
      devices: [],
      apps: []
    };
  },
  async beforeMount() {
    this.devices = await api.getDevices()
    this.$Progress.finish()
    this.loading = false
  }
};
</script>

