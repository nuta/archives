<template>
<dashboard-layout title="Devices">
  <div class="list">
    <div v-for="device in devices" class="element">
      <div class="title">
        <router-link :to="{ name: 'deviceSettings', params: { deviceName: device.name }}">
          {{ device.name }}
        </router-link>
      </div>

      <div class="rightside columns">
        <table class="metadata">
          <tr>
             <td><i class="fa fa-tag" aria-hidden="true"></i>Tag</td>
             <td>{{ device.tag || "(unspecified)" }}</td>
          </tr>
          <tr>
             <td><i class="fa fa-calendar" aria-hidden="true"></i>Registered at</td>
             <td>{{ device.created_at | strftime }}</td>
          </tr>
        </table>
      </div>
    </div>
  </div>
</dashboard-layout>
</template>

<script>
import api from "js/api";
import { strftime } from "js/filters";
import DashboardLayout from "layouts/dashboard";

export default {
  components: { DashboardLayout },
  filters: { strftime },
  data() {
    return {
      devices: [],
      apps: []
    };
  },
  async beforeMount() {
    this.devices = await api.getDevices()
  }
};
</script>

<style lang="scss" scoped>
</style>
