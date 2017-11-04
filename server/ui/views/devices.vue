<template>
<dashboard-layout title="Devices">
  <remote-content :loading="loading" :content="devices">
    <p slot="welcome-title">No devices yet!</p>

    <table slot="content" class="uk-table uk-table-divider">
      <thead>
        <tr>
          <th>Name</th>
          <th>Type</th>
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
          <td>{{ device.tag || "(unspecified)" }}</td>
          <td>{{ device.created_at | strftime }}</td>
          <td class="actions">
            <router-link :to="{ name: 'deviceSettings', params: { deviceName: device.name }}">
              <button class="uk-button">
                <span uk-icon="icon: forward"></span>
                Open Dashboard
              </button>
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

