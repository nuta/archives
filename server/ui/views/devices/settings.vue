<template>
<device-layout :title="deviceName" :device-name="deviceName">

  <remote-content :loading="loading" :content="device">
    <form slot="content" @submit.prevent="update" class="uk-form-horizontal">
      <div class="uk-margin">
        <label class="uk-form-label">Associated app</label>
        <div class="uk-form-controls">
          <select v-model="associatedTo" class="uk-select">
            <option value="" disabled selected>Not associated</option>
            <template v-for="app in apps">
              <option :value="app.name">{{ app.name }}</option>
            </template>
          </select>
        </div>
      </div>

      <div class="uk-margin">
        <label class="uk-form-label">Tag</label>
        <div class="uk-form-controls">
          <input type="text" v-model="tag" class="uk-input" placeholder="e.g. staging, group1, palo_alto">
        </div>
      </div>

      <input type="submit" value="Save" class="uk-button uk-button-primary uk-align-right">
    </form>
  </remote-content>
</device-layout>
</template>

<script>
import api from "js/api";
import RemoteContent from "components/remote-content";
import DeviceLayout from "layouts/device";

export default {
  components: { DeviceLayout, RemoteContent },
  data() {
    return {
      deviceName: app.$router.currentRoute.params.deviceName,
      device: null,
      apps: [],
      associatedTo: '',
      tag: '',
      loading: true
    }
  },
  methods: {
    update() {
      api.updateDevice(this.deviceName, {
        app: this.associatedTo || undefined,
        tag: this.tag || undefined
      })
    }
  },
  async beforeMount() {
    this.device = await api.getDevice(this.deviceName)
    this.apps = await api.getApps()
    this.associatedTo = this.device.app || ''

    this.loading = false
    this.$Progress.finish()
  }
};
</script>
