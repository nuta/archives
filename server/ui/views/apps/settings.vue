<template>
<app-layout path="appSettings" :app-name="appName">
  <remote-content :loading="loading" :content="app">
    <form slot="content" @submit.prevent="update" class="uk-form-horizontal">
      <div class="uk-margin">
        <label class="uk-form-label">API</label>
        <div class="uk-form-controls">
          <input type="text" v-model="appAPI" class="uk-input" disabled>
        </div>
      </div>

      <div class="uk-margin">
        <label class="uk-form-label">OS version</label>
        <div class="uk-form-controls">
          <select v-model="osVersion" class="uk-select">
            <template v-for="(release, version) in osReleases">
              <template v-for="(asset, os) in release">
                <option :value="version">{{ asset.title }}</option>
              </template>
            </template>
          </select>
        </div>
      </div>

      <input type="submit" value="Save" class="uk-button uk-button-primary uk-align-right">
    </form>
  </remote-content>
</app-layout>
</template>

<script>
import api from "js/api"
import RemoteContent from "components/remote-content"
import AppLayout from "layouts/app"

export default {
  components: { AppLayout, RemoteContent },
  data() {
    return {
      loading: true,
      appName: app.$router.currentRoute.params.appName,
      app: null,
      appAPI: null,
      osVersion: null,
      osReleases: null
    };
  },
  methods: {
    async update() {
      await api.updateApp(this.appName, { os_version: this.osVersion })
      this.$Notification.success('Updated settings.')
    }
  },
  async beforeMount() {
    this.app = await api.getApp(this.appName)
    this.appAPI = this.app.api
    this.osVersion = this.app.os_version
    this.osReleases = (await api.getOSReleases()).releases

    this.loading = false
    this.$Progress.finish()
  }
};
</script>

<style lang="scss" scoped>
</style>
