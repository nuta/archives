<template>
<dashboard-layout title="Devices">
  <div class="list">
    <div v-for="device in devices" class="element">
      <div class="title">
        <router-link :to="{ name: 'deviceOverview', params: { deviceName: device.name }}">
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

        <menu-button icon="fa-ellipsis-h" :elements="menuElements"
         @associateDevice="openAssociateDeviceModal(device.name)"></menu-button>
      </div>
    </div>
  </div>

  <modal :title="associateDeviceModal.title" :active="associateDeviceModal.active"
   @close="associateDeviceModal.active = false">
    <form @submit.prevent="associateDeviceToApp">
      <select v-model="associateDevice.appName">
        <template v-for="app in apps">
          <option :value="app.name">{{ app.name }}</option>
        </template>
      </select>

      <input type="submit" value="Associate">
    </form>
  </modal>
</dashboard-layout>
</template>

<script>
import api from "js/api";
import { strftime } from "js/filters";
import MenuButton from "components/menu-button";
import Modal from "components/modal";
import DashboardLayout from "layouts/dashboard";

export default {
  components: { DashboardLayout, Modal, MenuButton },
  filters: { strftime },
  data() {
    return {
      devices: [],
      apps: [],
      menuElements:[
          { event: 'associateDevice', title: 'Associate to an app' }
      ],
      associateDeviceModal: {
        title: "",
        active: false
      },
      associateDevice: {
        deviceName: "",
        appName: ""
      }
    };
  },
  methods: {
    openAssociateDeviceModal(deviceName) {
      this.associateDeviceModal.active = true;
      this.associateDeviceModal.title = `Associate ${deviceName} to an app`;
      this.associateDevice.deviceName = deviceName;
    },
    associateDeviceToApp() {
      api.associateDeviceToApp(this.associateDevice.deviceName,
                               this.associateDevice.appName);
    }
  },
  beforeMount() {
    api.getDevices().then(r => {
      this.devices = r.json;
    }, error => { notify("error", error); });

    api.getApps().then(r => {
      this.apps = r.json;
    }, error => { notify("error", error); });
  }
};
</script>

<style lang="scss" scoped>
</style>
