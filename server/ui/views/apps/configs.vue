<template>
<app-layout path="appConfigs" :app-name="appName">
  <div class="uk-container">
    <button class="uk-button uk-button-primary uk-align-right" uk-toggle="target: #create-store-modal">
      <span uk-icon="icon: plus"></span>
      Create a store
    </button>
  </div>

  <remote-content :loading="loading" :content="stores">
    <p slot="welcome-title">No stores (app scope) yet!</p>
    <p slot="welcome-description">
      App stores are one-way messages (server to device; readonly from device)
      such as configuration, display messages, and rump states (ON/OFF).
    </p>

    <table slot="content" class="uk-table uk-table-divider">
      <thead>
        <tr>
          <th>Key</th>
          <th>Type</th>
          <th>Value</th>
          <th></th>
        </tr>
      </thead>

      <tbody>
        <tr v-for="store in stores">
          <td>{{ store.key }}</td>
          <td>
            <select v-if="store.editing" class="uk-select" v-model="store.data_type">
              <option value="string">String</option>
              <option value="integer">Integer</option>
              <option value="float">Float</option>
              <option value="bool">Boolean</option>
            </select>
            <span v-else>{{ store.data_type }}</span>
          </td>

          <td class="uk-table-expand">
            <input v-if="store.editing" type="text" v-model="store.value" class="uk-input" placeholder="Value (e.g. 123, true, hello world!)">
            <code v-else>{{ store.value }}</code>
          </td>

          <td class="actions">
            <button v-if="store.editing" @click="updateConfig(store)" class="uk-button uk-button-primary">
              <i class="fa fa-check" aria-hidden="true"></i>
              Save
            </button>

            <button v-else @click="store.editing = !store.editing" class="uk-button uk-button-default">
              <i class="fa fa-pencil" aria-hidden="true"></i>
              Edit
            </button>

            <button v-if="store.editing" @click="store.editing = !store.editing" class="uk-button uk-button-default">
              <i class="fa fa-ban" aria-hidden="true"></i>
              Cancel
            </button>

            <button v-else @click="deleteConfig(store)" class="uk-button uk-button-danger">
              <i class="fa fa-trash" aria-hidden="true"></i>
            </button>
          </td>
        </tr>
      </tbody>
    </table>
  </remote-content>

  <div class="uk-flex-top" uk-modal id="create-store-modal">
    <div class="uk-modal-dialog">
      <div class="uk-modal-header">
        <button class="uk-modal-close-default" type="button" uk-close></button>
        <h2 class="uk-modal-title">Add a new store</h2>
      </div>

      <form @submit.prevent="createConfig(newStore)">
        <div class="uk-modal-body">
          <div class="uk-margin">
            <label class="uk-form-label">Key</label>
            <div class="uk-form-controls">
              <input type="text" v-model="newConfig.key" class="uk-input uk-form-width-large" placeholder="Key (e.g. message)">
            </div>
          </div>

          <div class="uk-margin">
            <label class="uk-form-label">Type</label>
            <div class="uk-form-controls">
              <select v-model="newConfig.dataType" class="uk-select uk-form-width-large">
                <option value="string">String</option>
                <option value="integer">Integer</option>
                <option value="float">Float</option>
                <option value="bool">Boolean</option>
              </select>
            </div>
          </div>

          <div class="uk-margin">
            <label class="uk-form-label">Value</label>
            <div class="uk-form-controls">
              <input type="text" v-model="newConfig.value" class="uk-input uk-form-width-large" placeholder="Value (e.g. 123, true, hello world!)">
            </div>
          </div>
        </div>

        <div class="uk-modal-footer">
          <input type="submit" class="uk-button uk-button-primary" value="Create">
        </div>
      </form>
    </div>
  </div>
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
      appName: app.$router.currentRoute.params.appName,
      stores: [],
      loading: true,
      newConfig: {
        key: "",
        dataType: "string",
        value: ""
      }
    }
  },
  methods: {
    async createConfig() {
      await api.updateAppConfig(
              this.appName,
              this.newConfig.key,
              this.newConfig.dataType,
              this.newConfig.value)

      await this.refreshAppConfigs()
      this.$Notification.success('Created a store.')
    },
    async updateConfig(store) {
      await api.updateAppConfig(
              this.appName,
              store.key,
              store.data_type,
              store.value)

      await this.refreshAppConfigs()
      this.$Notification.success('Updated a store.')
    },
    async deleteConfig(store) {
      await api.deleteAppConfig(this.appName, store.key)
      await this.refreshAppConfigs()
      this.$Notification.success('Deleted a store.')
    },
    async refreshAppConfigs() {
      this.stores = (await api.getAppConfigs(this.appName)).map(store => {
        store.editing = false
        return store
      })
    }
  },
  async beforeMount() {
    await this.refreshAppConfigs()
    this.$Progress.finish()
    this.loading = false
  }
}
</script>
