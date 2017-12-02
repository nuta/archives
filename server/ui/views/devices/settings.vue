<template>
<device-layout :title="deviceName" :device-name="deviceName">

  <remote-content :loading="loading" :content="device">
    <div slot="content">
      <form @submit.prevent="update" class="uk-form-horizontal">
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

        <div class="uk-margin">
          <label class="uk-form-label">sakura.io integration</label>
          <div class="uk-form-controls">
            <input type="text" v-model="sakuraio_module_token" class="uk-input" placeholder="The sakura.io communication module ID.">
          </div>
        </div>

        <input type="submit" value="Save" class="uk-button uk-button-primary uk-align-right">
      </form>

      <div class="uk-margin-xlarge-top">
        <h3>Credentials</h3>


        <table slot="content" class="uk-table">
          <tbody>
            <tr>
              <td>Device ID</td>
              <td><code>{{ device.device_id }}</code></td>
            </tr>
            <tr>
              <td>Device Secret</td>
              <td><code>{{ device.device_secret }}</code></td>
            </tr>
          </tbody>
        </table>
      </div>
    </div>
  </remote-content>


  <div class="uk-margin-xlarge-top">
    <h3>Reboot the device</h3>
    <p>Send a reboot command to {{ deviceName }}.</p>
    <button class="uk-button uk-button-danger" uk-toggle="target: #reboot-modal">
      Reboot
    </button>
  </div>


  <div class="uk-flex-top" uk-modal id="reboot-modal">
    <div class="uk-modal-dialog">
      <div class="uk-modal-header">
        <button class="uk-modal-close-default" type="button" uk-close></button>
        <h2 class="uk-modal-title">Reboot a device</h2>
      </div>
      <div class="uk-modal-body">
        Are you sure you want to reboot {{ deviceName }}?
      </div>
      <div class="uk-modal-footer uk-text-right">
          <button uk-toggle="target: #reboot-modal" class="uk-button uk-button-primary">
            Cancel
          </button>
          <button @click="reboot" class="uk-button uk-button-danger">
            Reboot {{ deviceName }}
          </button>
      </div>
    </div>
  </div>

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
      sakuraio_module_token: '',
      loading: true
    }
  },
  methods: {
    async update() {
      await api.updateDevice(this.deviceName, {
        app: this.associatedTo || undefined,
        tag: this.tag || undefined,
        sakuraio_module_token: this.sakuraio_module_token || undefined
      })

     this.$Notification.success('Updated settings.')
    },
    async reboot() {
      await api.rebootDevice(this.deviceName)
      UIkit.modal('#reboot-modal').hide()
      this.$Notification.success('Requested reboot.')
    }
  },
  async beforeMount() {
    this.device = await api.getDevice(this.deviceName)
    this.apps = await api.getApps()
    this.associatedTo = this.device.app || ''
    this.sakuraio_module_token = this.device.sakuraio_module_token || ''

    this.loading = false
    this.$Progress.finish()
  }
};
</script>
